#pragma once

#include <cstdint>
#include <array>
#include <map>
#include <cstddef>


struct Nucleotide {
  char Symbol;
  size_t Position;
  int ChromosomeNum;
  int GeneNum;
  bool IsMarked;
  char ServiceInfo;
};

const std::array<char, 4> symbols = {'A', 'T', 'G', 'C'};
const std::map<char, uint64_t> symbol_index = {{'A', 0 }, {'T', 1}, {'G', 2}, {'C', 3}};

struct CompactNucleotide {
  uint64_t Symbol:2;
  uint64_t Position:32;
  uint64_t ChromosomeNum:6;
  uint64_t GeneNum:15;
  uint64_t IsMarked:1;
  uint64_t ServiceInfo:8;
};

static_assert(sizeof(CompactNucleotide) <= 8, "The size of CompactNucleotide is greater than 8");

bool operator == (const Nucleotide& lhs, const Nucleotide& rhs);

CompactNucleotide Compress(const Nucleotide& n);
Nucleotide Decompress(const CompactNucleotide& cn);
