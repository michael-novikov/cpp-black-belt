#include <cstddef>


struct Nucleotide {
  char Symbol;
  size_t Position;
  int ChromosomeNum;
  int GeneNum;
  bool IsMarked;
  char ServiceInfo;
};


struct CompactNucleotide {
  // ...
};


bool operator == (const Nucleotide& lhs, const Nucleotide& rhs) {
  return (lhs.Symbol == rhs.Symbol)
      && (lhs.Position == rhs.Position)
      && (lhs.ChromosomeNum == rhs.ChromosomeNum)
      && (lhs.GeneNum == rhs.GeneNum)
      && (lhs.IsMarked == rhs.IsMarked)
      && (lhs.ServiceInfo == rhs.ServiceInfo);
}


CompactNucleotide Compress(const Nucleotide& n) {
  // ...
};


Nucleotide Decompress(const CompactNucleotide& cn) {
  // ...
}
