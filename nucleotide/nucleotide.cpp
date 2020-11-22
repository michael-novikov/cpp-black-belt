#include "nucleotide.h"


bool operator == (const Nucleotide& lhs, const Nucleotide& rhs) {
  return (lhs.Symbol == rhs.Symbol)
      && (lhs.Position == rhs.Position)
      && (lhs.ChromosomeNum == rhs.ChromosomeNum)
      && (lhs.GeneNum == rhs.GeneNum)
      && (lhs.IsMarked == rhs.IsMarked)
      && (lhs.ServiceInfo == rhs.ServiceInfo);
}


CompactNucleotide Compress(const Nucleotide& n) {
  return {
    .Symbol = symbol_index.at(n.Symbol),
    .Position = n.Position,
    .ChromosomeNum = static_cast<uint64_t>(n.ChromosomeNum),
    .GeneNum = static_cast<uint64_t>(n.GeneNum),
    .IsMarked = static_cast<uint64_t>(n.IsMarked ? 1 : 0),
    .ServiceInfo = static_cast<uint64_t>(n.ServiceInfo),
  };
};


Nucleotide Decompress(const CompactNucleotide& cn) {
  return {
    .Symbol = symbols[cn.Symbol],
    .Position = cn.Position,
    .ChromosomeNum = cn.ChromosomeNum,
    .GeneNum = cn.GeneNum,
    .IsMarked = (cn.IsMarked == 1),
    .ServiceInfo = static_cast<char>(cn.ServiceInfo),
  };
}
