#ifndef LLP_UTIL_H
#define LLP_UTIL_H

#include "llvm/Support/CommandLine.h"

// A ratio in [0., 1.] interval.
class Ratio {
    double Value = 0.0;

public:
    Ratio() = default;
    Ratio(double Value) : Value(Value) {}
    double getRatio() const { return Value; }
    void setRatio(double InValue) { Value = InValue; }
};

// 为了解析 Ratio 而对 CL 解析器进行扩展
namespace llvm {
    namespace cl {
        template <> class parser<Ratio> : public basic_parser<Ratio> {
        public:
            parser(Option &Opt) : basic_parser<Ratio>(Opt) {}
            virtual ~parser() = default;
            bool parse(Option &O, StringRef ArgName, StringRef &Arg, Ratio &Val);
            void printOptionDiff(const Option &O, Ratio const &V, OptionValue<Ratio> D, size_t GlobalWidth) const;
        }; // end template parser<Ratio>
    } // end namespace cl
} // end namespace llvm

#endif // UTIL_H