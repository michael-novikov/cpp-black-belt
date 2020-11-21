#include "yellow_pages.h"

#include "company.pb.h"
#include "signal.pb.h"

#include <algorithm>
#include <tuple>

using namespace std;
using namespace google::protobuf;

namespace YellowPages {

Company Merge(const Signals &signals, const Providers &providers) {
  if (signals.empty()) {
    return Company{};
  }

  vector<Signal> signals_with_company;
  copy_if(begin(signals), end(signals), back_inserter(signals_with_company),
          [](const Signal& signal) { return signal.has_company(); });

  Company res;
  auto descriptor = res.GetDescriptor();
  for (int i = 0; i < descriptor->field_count(); ++i) {
    auto field = descriptor->field(i);
    auto name = field->name();

    Signals companies_with_field;
    copy_if(begin(signals_with_company), end(signals_with_company), back_inserter(companies_with_field),
            [field] (const Signal& signal) {
              auto company = signal.company();
              return field->is_repeated() ? company.GetReflection()->FieldSize(company, field) : company.GetReflection()->HasField(company, field);
            });

    if (companies_with_field.empty()) {
      continue;
    }

    auto max_priority_signal =
        max_element(begin(companies_with_field), end(companies_with_field),
                    [&providers](const Signal &lhs, const Signal &rhs) {
                      return providers.at(lhs.provider_id()).priority() <
                             providers.at(rhs.provider_id()).priority();
                    });

    auto max_priority = providers.at(max_priority_signal->provider_id()).priority();

    companies_with_field.erase(
        remove_if(begin(companies_with_field), end(companies_with_field),
                  [max_priority, &providers](const Signal &signal) {
                    return providers.at(signal.provider_id()).priority() != max_priority;
                  }),
        end(companies_with_field)
    );

    auto res_reflection = res.GetReflection();
    if (field->is_repeated()) {
      for (const auto& company_signal : companies_with_field) {
        const auto company_to_merge = company_signal.company();
        auto reflection = company_to_merge.GetReflection();
        if (auto field_size = reflection->FieldSize(company_to_merge, field); field_size > 0) {
          for (int i = 0; i < field_size; ++i) {
            const auto& repeated_message = reflection->GetRepeatedMessage(company_to_merge, field, i);
            bool found = false;
            for (int j = 0; j < res_reflection->FieldSize(res, field); ++j) {
              if (res_reflection->GetRepeatedMessage(res, field, j).SerializeAsString() == repeated_message.SerializeAsString()) {
                found = true;
              }
            }
            if (!found) {
              res_reflection->AddMessage(&res, field)->MergeFrom(repeated_message);
            }
          }
        }
      }
    } else {
      const auto& company_to_merge = companies_with_field.begin()->company();
      auto reflection = company_to_merge.GetReflection();
      if (reflection->HasField(company_to_merge, field)) {
        const auto& field_to_merge = reflection->GetMessage(company_to_merge, field);
        res_reflection->MutableMessage(&res, field)->MergeFrom(field_to_merge);
      }
    }
  }

  return res;
}

} // namespace YellowPages
