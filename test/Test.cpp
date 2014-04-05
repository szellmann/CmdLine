// This file is distributed under the MIT license.
// See the LICENSE file for details.

#include "Support/CmdLine.h"
#include "Support/StringSplit.h"

#include "CmdLineQt.h"
#include "PrettyPrint.h"

#include <forward_list>
#include <functional>
#include <iostream>
#include <set>

using namespace support;

#define RETURN(...) -> decltype((__VA_ARGS__)) { return __VA_ARGS__; }

struct ForwardListInserter
{
    template <class C, class V>
    void operator ()(C& c, V&& v) const
    {
        c.insert_after(c.end(), std::forward<V>(v));
    }
};

namespace support
{
namespace cl
{

    template <class OptionT>
    void prettyPrint(std::ostream& stream, OptionWrapper<OptionT> const& option)
    {
        stream << option.name() << ":\n";
        stream << "  count = " << option.count() << "\n";
        stream << "  value = " << pretty(option.value());
    }

#if 1
    // Integrate forward_list into the command line library
    template <class T>
    struct Traits<std::forward_list<T>> : BasicTraits<T, ForwardListInserter>
    {
    };
#endif

} // namespace cl
} // namespace support

struct WFlagParser
{
    void operator ()(StringRef name, StringRef /*arg*/, bool& value) const
    {
        value = !name.starts_with("Wno-");
    }
};

template <class... Args>
auto makeWFlag(Args&&... args)
RETURN(
    cl::makeOption<bool>(WFlagParser(), std::forward<Args>(args)..., cl::ArgDisallowed, cl::ZeroOrMore)
)

int main(int argc, char* argv[])
{
    //----------------------------------------------------------------------------------------------

    cl::CmdLine cmd;

                    //------------------------------------------------------------------------------

    auto help = cl::makeOption<std::string>(
        cl::Parser<>(), cmd, "help",
        cl::ArgName("option"),
        cl::ArgOptional
        );

                    //------------------------------------------------------------------------------

    double y = -1.0;

    auto y_ref = cl::makeOption<double&>(
        cl::Parser<>(), cmd, "y",
        cl::ArgName("float"),
        cl::ArgRequired,
        cl::init(y)
        );

                    //------------------------------------------------------------------------------

    auto g  = cl::makeOption<bool>(cl::Parser<>(), cmd, "g", cl::Grouping, cl::ArgDisallowed, cl::ZeroOrMore);
    auto h  = cl::makeOption<bool>(cl::Parser<>(), cmd, "h", cl::Grouping, cl::ArgDisallowed, cl::ZeroOrMore);
    auto gh = cl::makeOption<bool>(cl::Parser<>(), cmd, "gh", cl::Prefix, cl::ArgRequired);

                    //------------------------------------------------------------------------------

    auto z = cl::makeOption<std::set<int>>(
        cl::Parser<>(), cmd, "z",
        cl::ArgName("int"),
        cl::ArgRequired,
        cl::CommaSeparated,
        cl::ZeroOrMore
        );

                    //------------------------------------------------------------------------------

    auto I = cl::makeOption<std::vector<std::pair<std::string, size_t>>>(
        [&](StringRef /*name*/, StringRef arg, std::pair<std::string, size_t>& value) {
            value = { arg.str(), cmd.index() };
        },
        cmd, "I",
        cl::ArgName("dir"),
        cl::ArgRequired,
        cl::Prefix,
        cl::ZeroOrMore
        );

                    //------------------------------------------------------------------------------

    auto files = cl::makeOption<std::vector<std::string>>(
        cl::Parser<>(),
        cmd, "files",
        cl::Positional,
        cl::ZeroOrMore
        );

                    //------------------------------------------------------------------------------

    enum OptimizationLevel {
        OL_None,
        OL_Trivial,
        OL_Default,
        OL_Expensive
    };

    auto optParser = cl::MapParser<OptimizationLevel>({
        { "O0", OL_None      },
        { "O1", OL_Trivial   },
        { "O2", OL_Default   },
        { "O3", OL_Expensive },
    });

    auto opt = cl::makeOption<OptimizationLevel>(
        std::ref(optParser),
        cmd,
        cl::ArgDisallowed,
        cl::ArgName("optimization level"),
        cl::init(OL_None),
        cl::Required
        );

                    //------------------------------------------------------------------------------

    enum Simpson {
        Homer, Marge, Bart, Lisa, Maggie, SideshowBob
    };

    auto simpson = cl::makeOption<Simpson>(
        {
            { "homer",        Homer       },
            { "marge",        Marge       },
            { "bart",         Bart        },
            { "el barto",     Bart        },
            { "lisa",         Lisa        },
            { "maggie",       Maggie      },
//          { "sideshow bob", SideshowBob },
        },
        cmd, "simpson",
        cl::ArgRequired,
        cl::init(SideshowBob)
        );

                    //------------------------------------------------------------------------------

    auto f = cl::makeOption<std::map<std::string, int>, cl::Traits/*default*/>(
        [](StringRef name, StringRef arg, std::pair<std::string, int>& value)
        {
            auto p = strings::split_once(arg, ":");

            cl::Parser<>()(name, p.first, value.first);
            cl::Parser<>()(name, p.second, value.second);
        },
        cmd, "f",
        cl::ArgName("string:int"),
        cl::ArgRequired,
        cl::CommaSeparated
        );

                    //------------------------------------------------------------------------------

    auto debug_level = cl::makeOption<int>(
        cl::Parser<>(),
        cmd, "debug-level|d",
        cl::ArgRequired,
        cl::Optional
        );

                    //------------------------------------------------------------------------------

    auto Wsign_conversion = makeWFlag(cmd, "Wsign-conversion|Wno-sign-conversion");

    auto Wsign_compare = makeWFlag(cmd, "Wsign-compare|Wno-sign-compare");

                    //------------------------------------------------------------------------------

    auto targets = cl::makeOption<std::set<std::string>, cl::ScalarType>(
        [](StringRef name, StringRef arg, std::set<std::string>& value)
        {
            if (name.starts_with("without-"))
                value.erase(arg.str());
            else
                value.insert(arg.str());
        },
        cmd, "without-|with-",
        cl::ArgName("target"),
        cl::ArgRequired,
        cl::CommaSeparated,
        cl::Prefix,
        cl::ZeroOrMore
    );

                    //------------------------------------------------------------------------------

#if 1
    auto x_list = cl::makeOption<std::forward_list<int>>(cl::Parser<>(), cmd, "x_list");
#endif

    //----------------------------------------------------------------------------------------------

    try
    {
        cmd.parse({ argv + 1, argv + argc });
    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        return -1;
    }

    //----------------------------------------------------------------------------------------------

    std::cout << pretty(debug_level) << std::endl;
    std::cout << pretty(f) << std::endl;
    std::cout << pretty(files) << std::endl;
    std::cout << pretty(g) << std::endl;
    std::cout << pretty(gh) << std::endl;
    std::cout << pretty(h) << std::endl;
    std::cout << pretty(I) << std::endl;
    std::cout << pretty(opt) << std::endl;
    std::cout << pretty(simpson) << std::endl;
    std::cout << pretty(targets) << std::endl;
    std::cout << pretty(Wsign_compare) << std::endl;
    std::cout << pretty(Wsign_conversion) << std::endl;
    std::cout << pretty(y_ref) << std::endl;
    std::cout << pretty(z) << std::endl;

    //----------------------------------------------------------------------------------------------

    return 0;
}
