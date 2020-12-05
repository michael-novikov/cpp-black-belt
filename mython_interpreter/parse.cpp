#include "parse.h"
#include "statement.h"
#include "lexer.h" // Добавьте в проект вашу реализацию лексического анализатора языка Mython
#include "comparators.h"

#include <algorithm>
#include <string>
#include <cctype>
#include <vector>
#include <optional>

using namespace std;

namespace TokenType = Parse::TokenType;

namespace {
bool operator == (const Parse::Token& token, char c) {
  auto p = token.TryAs<TokenType::Char>();
  return p && p->value == c;
}

bool operator != (const Parse::Token& token, char c) {
  return !(token == c);
}

}

class Parser {
public:
  explicit Parser(Parse::Lexer& lexer) : lexer(lexer) {
  }

  // Program -> eps
  //          | Statement \n Program
  unique_ptr<Ast::Statement> ParseProgram() {
    auto result = make_unique<Ast::Compound>();
    while (!lexer.CurrentToken().Is<TokenType::Eof>()) {
      result->AddStatement(ParseStatement());
    }

    return result;
  }

private:
  Parse::Lexer& lexer;
  Runtime::Closure declared_classes;

  // Suite -> NEWLINE INDENT (Statement)+ DEDENT
  unique_ptr<Ast::Statement> ParseSuite() {
    lexer.Expect<TokenType::Newline>();
    lexer.ExpectNext<TokenType::Indent>();

    lexer.NextToken();

    auto result = make_unique<Ast::Compound>();
    while (!lexer.CurrentToken().Is<TokenType::Dedent>()) {
      result->AddStatement(ParseStatement());
    }

    lexer.Expect<TokenType::Dedent>();
    lexer.NextToken();

    return result;
  }

  // Methods -> [def id(Params) : Suite]*
  vector<Runtime::Method> ParseMethods() {
    vector<Runtime::Method> result;

    while (lexer.CurrentToken().Is<TokenType::Def>()) {
      Runtime::Method m;

      m.name = lexer.ExpectNext<TokenType::Id>().value;
      lexer.ExpectNext<TokenType::Char>('(');

      if (lexer.NextToken().Is<TokenType::Id>()) {
        m.formal_params.push_back(lexer.Expect<TokenType::Id>().value);
        while (lexer.NextToken() == ',') {
          m.formal_params.push_back(lexer.ExpectNext<TokenType::Id>().value);
        }
      }

      lexer.Expect<TokenType::Char>(')');
      lexer.ExpectNext<TokenType::Char>(':');
      lexer.NextToken();

      m.body = ParseSuite();

      result.push_back(std::move(m));
    }
    return result;
  }

  // ClassDefinition -> Id ['(' Id ')'] : new_line indent MethodList dedent
  unique_ptr<Ast::Statement> ParseClassDefinition() {
    string class_name = lexer.Expect<TokenType::Id>().value;

    lexer.NextToken();

    const Runtime::Class* base_class = nullptr;
    if (lexer.CurrentToken() == '(') {
      auto name = lexer.ExpectNext<TokenType::Id>().value;
      lexer.ExpectNext<TokenType::Char>(')');
      lexer.NextToken();

      if (auto it = declared_classes.find(name); it == declared_classes.end()) {
        throw ParseError("Base class " + name + " not found for class " + class_name);
      } else {
        base_class = static_cast<const Runtime::Class*>(it->second.Get());
      }
    }

    lexer.Expect<TokenType::Char>(':');
    lexer.ExpectNext<TokenType::Newline>();
    lexer.ExpectNext<TokenType::Indent>();
    lexer.ExpectNext<TokenType::Def>();
    vector<Runtime::Method> methods = ParseMethods();

    lexer.Expect<TokenType::Dedent>();
    lexer.NextToken();

    auto [it, inserted] = declared_classes.insert({
      class_name,
      ObjectHolder::Own(Runtime::Class(class_name, std::move(methods), base_class))
    });

    if (!inserted) {
      throw ParseError("Class " + class_name + " already exists");
    }

    return make_unique<Ast::ClassDefinition>(it->second);
  }

  vector<string> ParseDottedIds() {
    vector<string> result(1, lexer.Expect<TokenType::Id>().value);

    while (lexer.NextToken() == '.') {
      result.push_back(lexer.ExpectNext<TokenType::Id>().value);
    }

    return result;
  }

  //  AssgnOrCall -> DottedIds = Expr
  //               | DottedIds '(' ExprList ')'
  unique_ptr<Ast::Statement> ParseAssignmentOrCall() {
    lexer.Expect<TokenType::Id>();

    vector<string> id_list = ParseDottedIds();
    string last_name = id_list.back();
    id_list.pop_back();

    if (lexer.CurrentToken() == '=') {
      lexer.NextToken();

      if (id_list.empty()) {
        return make_unique<Ast::Assignment>(std::move(last_name), ParseTest());
      } else {
        return make_unique<Ast::FieldAssignment>(
          Ast::VariableValue{std::move(id_list)}, std::move(last_name), ParseTest()
        );
      }
    } else {
      lexer.Expect<TokenType::Char>('(');
      lexer.NextToken();

      if (id_list.empty()) {
        throw ParseError("Mython doesn't support functions, only methods: " + last_name);
      }


      vector<unique_ptr<Ast::Statement>> args;
      if (lexer.CurrentToken() != ')') {
        args = ParseTestList();
      }
      lexer.Expect<TokenType::Char>(')');
      lexer.NextToken();

      return make_unique<Ast::MethodCall>(
        make_unique<Ast::VariableValue>(std::move(id_list)),
        std::move(last_name),
        std::move(args)
      );
    }
  }

  // Expr -> Adder ['+'/'-' Adder]*
  unique_ptr<Ast::Statement> ParseExpression() {
    unique_ptr<Ast::Statement> result = ParseAdder();
    while (lexer.CurrentToken() == '+' || lexer.CurrentToken() == '-') {
      char op = lexer.CurrentToken().As<TokenType::Char>().value;
      lexer.NextToken();

      if (op == '+') {
        result = make_unique<Ast::Add>(std::move(result), ParseAdder());
      } else {
        result = make_unique<Ast::Sub>(std::move(result), ParseAdder());
      }
    }
    return result;
  }

  // Adder -> Mult ['*'/'/' Mult]*
  unique_ptr<Ast::Statement> ParseAdder() {
    unique_ptr<Ast::Statement> result = ParseMult();
    while (lexer.CurrentToken() == '*' || lexer.CurrentToken() == '/') {
      char op = lexer.CurrentToken().As<TokenType::Char>().value;
      lexer.NextToken();

      if (op == '*') {
        result = make_unique<Ast::Mult>(std::move(result), ParseMult());
      } else {
        result = make_unique<Ast::Div>(std::move(result), ParseMult());
      }
    }
    return result;
  }

  // Mult -> '(' Expr ')'
  //       | NUMBER
  //       | '-' Mult
  //       | STRING
  //       | NONE
  //       | TRUE
  //       | FALSE
  //       | DottedIds '(' ExprList ')'
  //       | DottedIds
  unique_ptr<Ast::Statement> ParseMult() {
    if (lexer.CurrentToken() == '(') {
      lexer.NextToken();
      auto result = ParseTest();
      lexer.Expect<TokenType::Char>(')');
      lexer.NextToken();
      return result;
    } else if (lexer.CurrentToken() == '-') {
      lexer.NextToken();
      return make_unique<Ast::Mult>(
        ParseMult(),
        make_unique<Ast::NumericConst>(-1)
      );
    } else if (auto num = lexer.CurrentToken().TryAs<TokenType::Number>()) {
      int result = num->value;
      lexer.NextToken();
      return make_unique<Ast::NumericConst>(result);
    } else if (auto str = lexer.CurrentToken().TryAs<TokenType::String>()) {
      string result = str->value;
      lexer.NextToken();
      return make_unique<Ast::StringConst>(std::move(result));
    } else if (lexer.CurrentToken().Is<TokenType::True>()) {
      lexer.NextToken();
      return make_unique<Ast::BoolConst>(Runtime::Bool(true));
    } else if (lexer.CurrentToken().Is<TokenType::False>()) {
      lexer.NextToken();
      return make_unique<Ast::BoolConst>(Runtime::Bool(false));
    } else if (lexer.CurrentToken().Is<TokenType::None>()) {
      lexer.NextToken();
      return make_unique<Ast::None>();
    } else {
      vector<string> names = ParseDottedIds();

      if (lexer.CurrentToken() == '(') {
        // various calls
        vector<unique_ptr<Ast::Statement>> args;
        if (lexer.NextToken() != ')') {
          args = ParseTestList();
        }
        lexer.Expect<TokenType::Char>(')');
        lexer.NextToken();

        auto method_name = names.back();
        names.pop_back();

        if (!names.empty()) {
          return make_unique<Ast::MethodCall>(
            make_unique<Ast::VariableValue>(std::move(names)),
            std::move(method_name),
            std::move(args)
          );
        } else if (auto it = declared_classes.find(method_name); it != end(declared_classes)) {
          return make_unique<Ast::NewInstance>(
            static_cast<const Runtime::Class&>(*it->second), std::move(args)
          );
        } else if (method_name == "str") {
          if (args.size() != 1) {
            throw ParseError("Function str takes exactly one argument");
          }
          return make_unique<Ast::Stringify>(std::move(args.front()));
        } else {
          throw ParseError("Unknown call to " + method_name + "()");
        }
      } else {
        return make_unique<Ast::VariableValue>(std::move(names));
      }
    }
  }

  vector<unique_ptr<Ast::Statement>> ParseTestList() {
    vector<unique_ptr<Ast::Statement>> result;
    result.push_back(ParseTest());

    while (lexer.CurrentToken() == ',') {
      lexer.NextToken();
      result.push_back(ParseTest());
    }
    return result;
  }

  // Condition -> if LogicalExpr: Suite [else: Suite]
  unique_ptr<Ast::Statement> ParseCondition() {
    lexer.Expect<TokenType::If>();
    lexer.NextToken();

    auto condition = ParseTest();

    lexer.Expect<TokenType::Char>(':');
    lexer.NextToken();

    auto if_body = ParseSuite();

    unique_ptr<Ast::Statement> else_body;
    if (lexer.CurrentToken().Is<TokenType::Else>()) {
      lexer.ExpectNext<TokenType::Char>(':');
      lexer.NextToken();
      else_body = ParseSuite();
    }

    return make_unique<Ast::IfElse>(std::move(condition), std::move(if_body), std::move(else_body));
  }

  // LogicalExpr -> AndTest [OR AndTest]
  // AndTest -> NotTest [AND NotTest]
  // NotTest -> [NOT] NotTest
  //          | Comparison
  unique_ptr<Ast::Statement> ParseTest() {
    auto result = ParseAndTest();
    while (lexer.CurrentToken().Is<TokenType::Or>()) {
      lexer.NextToken();
      result = make_unique<Ast::Or>(std::move(result), ParseAndTest());
    }
    return result;
  }

  unique_ptr<Ast::Statement> ParseAndTest() {
    auto result = ParseNotTest();
    while (lexer.CurrentToken().Is<TokenType::And>()) {
      lexer.NextToken();
      result = make_unique<Ast::And>(std::move(result), ParseNotTest());
    }
    return result;
  }

  unique_ptr<Ast::Statement> ParseNotTest() {
    if (lexer.CurrentToken().Is<TokenType::Not>()) {
      lexer.NextToken();
      return make_unique<Ast::Not>(ParseNotTest());
    } else {
      return ParseComparison();
    }
  }

  // Comparison -> Expr [COMP_OP Expr]
  unique_ptr<Ast::Statement> ParseComparison() {
    auto result = ParseExpression();

    const auto tok = lexer.CurrentToken();

    if (tok == '<') {
      lexer.NextToken();
      return make_unique<Ast::Comparison>(Runtime::Less, std::move(result), ParseExpression());
    } else if (tok == '>') {
      lexer.NextToken();
      return make_unique<Ast::Comparison>(Runtime::Greater, std::move(result), ParseExpression());
    } else if (tok.Is<TokenType::Eq>()) {
      lexer.NextToken();
      return make_unique<Ast::Comparison>(Runtime::Equal, std::move(result), ParseExpression());
    } else if (tok.Is<TokenType::NotEq>()) {
      lexer.NextToken();
      return make_unique<Ast::Comparison>(Runtime::NotEqual, std::move(result), ParseExpression());
    } else if (tok.Is<TokenType::LessOrEq>()) {
      lexer.NextToken();
      return make_unique<Ast::Comparison>(Runtime::LessOrEqual, std::move(result), ParseExpression());
    } else if (tok.Is<TokenType::GreaterOrEq>()) {
      lexer.NextToken();
      return make_unique<Ast::Comparison>(Runtime::GreaterOrEqual, std::move(result), ParseExpression());
    } else {
      return result;
    }
  }

  //Statement -> SimpleStatement Newline
  //           | class ClassDefinition
  //           | if Condition
  unique_ptr<Ast::Statement> ParseStatement() {
    const auto& tok = lexer.CurrentToken();

    if (tok.Is<TokenType::Class>()) {
      lexer.NextToken();
      return ParseClassDefinition();
    } else if (tok.Is<TokenType::If>()) {
      return ParseCondition();
    } else {
      auto result = ParseSimpleStatement();
      lexer.Expect<TokenType::Newline>();
      lexer.NextToken();
      return result;
    }
  }

  //StatementBody -> return Expression
  //               | print ExpressionList
  //               | AssignmentOrCall
  unique_ptr<Ast::Statement> ParseSimpleStatement() {
    const auto& tok = lexer.CurrentToken();

    if (tok.Is<TokenType::Return>()) {
      lexer.NextToken();
      return make_unique<Ast::Return>(ParseTest());
    } else if (tok.Is<TokenType::Print>()) {
      lexer.NextToken();
      vector<unique_ptr<Ast::Statement>> args;
      if (!lexer.CurrentToken().Is<TokenType::Newline>()) {
        args = ParseTestList();
      }
      return make_unique<Ast::Print>(std::move(args));
    } else {
      return ParseAssignmentOrCall();
    }
  }
};

unique_ptr<Ast::Statement> ParseProgram(Parse::Lexer& lexer) {
  return Parser{lexer}.ParseProgram();
}
