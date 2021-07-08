//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "MSIXWindows.hpp"
#include "AppxPackaging.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <initializer_list>
#include <algorithm>
#include <functional>
#include <sstream>

#define TOOL_HELP_COMMAND_STRING "-?"

struct Invocation;

// Describes an option to a command that the user may specify.
struct Option
{
    // Constructor for flags; they can't be required and don't take parameters.
    Option(std::string name, std::string help) :
        Name(std::move(name)), Required(false), ParameterCount(0), Help(std::move(help))
    {}

    Option(std::string name, std::string help, bool required, size_t parameterCount, std::string parameterName) :
        Name(std::move(name)), Required(required), ParameterCount(parameterCount), Help(std::move(help)), ParameterName(std::move(parameterName))
    {}

    bool operator==(const std::string& rhs) const {
        return Name == rhs;
    }

    std::string Name;
    std::string Help;
    bool        Required;
    size_t      ParameterCount;
    std::string ParameterName;
};

// Describes a command that the user may specify.
struct Command
{
    using InvocationFunc = std::function<int(const Invocation&)>;

    Command(std::string name, std::string help, std::vector<Option> options) :
        Name(std::move(name)), Help(std::move(help)), Options(std::move(options))
    {}

    bool operator==(const std::string& rhs) const {
        return Name == rhs;
    }

    // Limit strings to 76 characters
    void SetDescription(std::vector<std::string>&& description)
    {
        Description = std::move(description);
    }

    void SetInvocationFunc(InvocationFunc func)
    {
        Invoke = func;
    }

    void PrintHelpText(const Invocation& invocation) const;

    std::string         Name;
    std::string         Help;
    std::vector<Option> Options;
    std::vector<std::string>    Description;
    InvocationFunc      Invoke = nullptr;
};

// Tracks the state of the current parse operation.
struct Invocation
{
    bool Parse(const std::vector<Command>& commands, int argc, char* argv[]) try
    {
        if (argc < 1)
        {
            error = "Unexpected; no arguments provided, not even executable name";
            return false;
        }

        toolName = argv[0];

        char const* commandString = nullptr;
        if (argc == 1)
        {
            // No arguments implies help
            commandString = TOOL_HELP_COMMAND_STRING;
        }
        else
        {
            commandString = argv[1];
        }

        // Find command, only happens once, at the beginning.
        auto commandItr = std::find(commands.begin(), commands.end(), commandString);

        if (commandItr == commands.end())
        {
            error = "Unrecognized command: ";
            error += commandString;
            return false;
        }

        command = &*commandItr;

        // Parse all parameters, ensuring that parameter count is met
        for (int index = 2; index < argc; ++index)
        {
            char const* optionString = argv[index];
            auto option = std::find(command->Options.begin(), command->Options.end(), optionString);

            if (option == command->Options.end())
            {
                error = "Unrecognized option: ";
                error += optionString;
                return false;
            }

            std::vector<std::string> params;
            for (size_t i = 0; i < option->ParameterCount; ++i)
            {
                if (++index == argc) {
                    error = "Not enough parameters for option: ";
                    error += optionString;
                    return false;
                }
                params.emplace_back(argv[index]);
            }

            options.emplace_back(*option, std::move(params));
        }

        // Ensure that all required parameters are present, unless help was requested
        if (!IsOptionPresent(TOOL_HELP_COMMAND_STRING))
        {
            for (const Option& option : command->Options)
            {
                if (option.Required)
                {
                    if (!IsOptionPresent(option.Name))
                    {
                        error = "Required option missing: ";
                        error += option.Name;
                        return false;
                    }
                }
            }
        }

        return true;
    }
    catch (const std::exception& exc)
    {
        error = "Exception thrown during Parse: ";
        error += exc.what();
        return false;
    }

    int Run() const try
    {
        if (!command)
        {
            error = "Unexpected; command should be set if Parse succeeded";
            return -1;
        }

        // Check for help option and have command print it
        if (IsOptionPresent(TOOL_HELP_COMMAND_STRING))
        {
            command->PrintHelpText(*this);
            return 0;
        }

        return command->Invoke(*this);
    }
    catch (const std::exception& exc)
    {
        error = "Exception thrown during Run: ";
        error += exc.what();
        return -1;
    }

    const std::string& GetToolName() const { return toolName; }

    const std::string& GetErrorText() const { return error; }

    const Command* GetParsedCommand() const { return command; }

    bool IsOptionPresent(const std::string& name) const
    {
        return (GetInvokedOption(name) != nullptr);
    }

    const std::string& GetOptionValue(const std::string& name) const
    {
        const InvokedOption* opt = GetInvokedOption(name);
        if (!opt)
        {
            throw std::runtime_error("Expected option not present");
        }
        if (opt->option.ParameterCount != 1)
        {
            throw std::runtime_error("Given option does not take exactly one parameter");
        }
        return opt->params[0];
    }

private:
    mutable std::string error;
    std::string         toolName;
    const Command*      command = nullptr;

    struct InvokedOption
    {
        InvokedOption(const Option& o, std::vector<std::string>&& p) : option(o), params(std::move(p)) {}

        bool operator==(const std::string& rhs) const {
            return option.Name == rhs;
        }

        const Option& option;
        std::vector<std::string> params;
    };

    std::vector<InvokedOption> options;

    const InvokedOption* GetInvokedOption(const std::string& name) const
    {
        auto option = std::find(options.begin(), options.end(), name);
        return (option == options.end() ? nullptr : &*option);
    }
};

void Command::PrintHelpText(const Invocation& invocation) const
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "---------------" << std::endl;

    std::cout << "    " << invocation.GetToolName() << ' ' << Name;

    bool areOptionalOptionsPresent = false;
    for (const Option& option : Options)
    {
        if (option.Required)
        {
            std::cout << ' ' << option.Name;

            for (size_t i = 0; i < option.ParameterCount; ++i)
            {
                std::cout << " <" << option.ParameterName << '>';
            }
        }
        else
        {
            areOptionalOptionsPresent = true;
        }
    }

    if (areOptionalOptionsPresent)
    {
        std::cout << " [options]";
    }
    std::cout << std::endl;

    if (!Description.empty())
    {
        std::cout << std::endl;
        std::cout << "Description:" << std::endl;
        std::cout << "---------------" << std::endl;

        for (const std::string& line : Description)
        {
            std::cout << "    " << line << std::endl;
        }
    }

    if (!Options.empty())
    {
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "---------------" << std::endl;

        for (const Option& option : Options)
        {
            std::cout << "    " << option.Name;

            if (option.ParameterCount)
            {
                std::cout << " <" << option.ParameterName << '>';

                if (option.ParameterCount > 1)
                {
                    std::cout << " [x" << option.ParameterCount << ']';
                }
            }
            else
            {
                std::cout << " [Flag]";
            }

            if (option.Required)
            {
                std::cout << " {Required}";
            }
            std::cout << std::endl;

            std::cout << "        " << option.Help << std::endl;
        }
    }
}

LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb)  { return std::malloc(cb); }

class Text
{
public:
    char** operator&() { return &content; }
    ~Text() { Cleanup(); }

    char* content = nullptr;
    protected:    
    void Cleanup() { if (content) { std::free(content); content = nullptr; } }
};

template <typename EnumType>
std::underlying_type_t<EnumType> asut(EnumType e)
{
    return static_cast<std::underlying_type_t<EnumType>>(e);
}

template <typename EnumType>
std::enable_if_t<std::is_enum<EnumType>::value, EnumType> operator|=(EnumType& a, EnumType b)
{
    a = static_cast<EnumType>(asut(a) | asut(b));
    return a;
}

MSIX_PACKUNPACK_OPTION GetPackUnpackOptionBase(const Invocation& invocation)
{
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE;

    if (invocation.IsOptionPresent("-pfn"))
    {
        packUnpack |= MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER;
    }

    return packUnpack;
}

MSIX_PACKUNPACK_OPTION GetPackUnpackOptionForPackage(const Invocation& invocation)
{
    MSIX_PACKUNPACK_OPTION packUnpack = GetPackUnpackOptionBase(invocation);

    if (invocation.IsOptionPresent("-pfn-flat"))
    {
        packUnpack |= MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER;
    }

    return packUnpack;
}

MSIX_PACKUNPACK_OPTION GetPackUnpackOptionForBundle(const Invocation& invocation)
{
    MSIX_PACKUNPACK_OPTION packUnpack = GetPackUnpackOptionBase(invocation);

    if (invocation.IsOptionPresent("-pfn-flat"))
    {
        packUnpack |= MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_UNPACKWITHFLATSTRUCTURE;
    }

    return packUnpack;
}

MSIX_VALIDATION_OPTION GetValidationOption(const Invocation& invocation)
{
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;

    if (invocation.IsOptionPresent("-ac"))
    {
        validation |= MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    }

    if (invocation.IsOptionPresent("-ss"))
    {
        validation |= MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    }

    return validation;
}

MSIX_APPLICABILITY_OPTIONS GetApplicabilityOption(const Invocation& invocation)
{
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_FULL;

    if (invocation.IsOptionPresent("-sl"))
    {
        applicability |= MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE;
    }

    if (invocation.IsOptionPresent("-sp"))
    {
        applicability |= MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPPLATFORM;
    }

    if (invocation.IsOptionPresent("-extract-all"))
    {
        applicability = static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_NONE);
    }

    return applicability;
}

MSIX_BUNDLE_OPTIONS GetBundleOptions(const Invocation& invocation)
{
    MSIX_BUNDLE_OPTIONS bundleOptions = MSIX_BUNDLE_OPTIONS::MSIX_OPTION_NONE;

    if (invocation.IsOptionPresent("-v"))
    {
        bundleOptions |= MSIX_BUNDLE_OPTIONS::MSIX_OPTION_VERBOSE;
    }

    if (invocation.IsOptionPresent("-o"))
    {
        bundleOptions |= MSIX_BUNDLE_OPTIONS::MSIX_OPTION_OVERWRITE;
    }

    if (invocation.IsOptionPresent("-no"))
    {
        bundleOptions |= MSIX_BUNDLE_OPTIONS::MSIX_OPTION_NOOVERWRITE;
    }

    if (invocation.IsOptionPresent("-bv"))
    {
        bundleOptions |= MSIX_BUNDLE_OPTIONS::MSIX_OPTION_VERSION;
    }

    if (invocation.IsOptionPresent("-fb"))
    {
        bundleOptions |= MSIX_BUNDLE_OPTIONS::MSIX_BUNDLE_OPTION_FLATBUNDLE;
    }

    if (invocation.IsOptionPresent("-mo"))
    {
        bundleOptions |= MSIX_BUNDLE_OPTIONS::MSIX_BUNDLE_OPTION_BUNDLEMANIFESTONLY;
    }

    return bundleOptions;
}

#pragma region Commands

Command CreateHelpCommand(const std::vector<Command>& commands)
{
    Command result{ TOOL_HELP_COMMAND_STRING, "Displays this help text.", {} };

    result.SetInvocationFunc([&commands](const Invocation& invocation)
        {
            std::cout << std::endl;
            std::cout << "Usage:" << std::endl;
            std::cout << "---------------" << std::endl;
            std::cout << "    " << invocation.GetToolName() << " <command> [options] " << std::endl;
            std::cout << std::endl;
            std::cout << "Commands:" << std::endl;
            std::cout << "---------------" << std::endl;

            for (const auto& c : commands)
            {
                std::cout << "    " << std::left << std::setfill(' ') << std::setw(10) <<
                    c.Name << "--  " << c.Help << std::endl;
            }

            std::cout << std::endl;
            std::cout << "For help with a specific command, enter " << invocation.GetToolName() << " <command> -?" << std::endl;

            return 0;
        });

    return result;
}

Command CreateUnpackCommand()
{
    Command result{ "unpack", "Unpack files from a package to disk",
        {
            Option{ "-p", "Input package file path.", true, 1, "package" },
            Option{ "-d", "Output directory path.", true, 1, "directory" },
            Option{ "-pfn", "Unpacks all files to a subdirectory under the output path, named after the package full name." },
            Option{ "-ac", "Allows any certificate. By default the signature origin must be known." },
            Option{ "-ss", "Skips enforcement of signed packages. By default packages must be signed." },
            // Identical behavior as -pfn. This option was created to create parity with unbundle's -pfn-flat option so that IT pros
            // creating packages for app attach only need to be aware of a single option.
            Option{ "-pfn-flat", "Same behavior as -pfn for packages." },
            Option{ TOOL_HELP_COMMAND_STRING, "Displays this help text." },
        }
    };

    result.SetDescription({
        "Extracts all files within an app package at the input <package> name to the",
        "specified output <directory>. The output has the same directory structure ",
        "as the package. If <package> is a bundle, it extract its contests with full",
        "applicability validations and its packages will be unpacked in a directory ",
        "named as the package full name.",
        });

    result.SetInvocationFunc([](const Invocation& invocation)
        {
            return UnpackPackage(
                GetPackUnpackOptionForPackage(invocation),
                GetValidationOption(invocation),
                const_cast<char*>(invocation.GetOptionValue("-p").c_str()),
                const_cast<char*>(invocation.GetOptionValue("-d").c_str()));
        });

    return result;
}

Command CreateUnbundleCommand()
{
    Command result{ "unbundle", "Unpack files from a bundle to disk",
        {
            Option{ "-p", "Input bundle file path.", true, 1, "bundle" },
            Option{ "-d", "Output directory path.", true, 1, "directory" },
            Option{ "-pfn", "Unpacks all files to a subdirectory under the output path, named after the package full name." },
            Option{ "-ac", "Allows any certificate. By default the signature origin must be known." },
            Option{ "-ss", "Skips enforcement of signed packages. By default packages must be signed." },
            Option{ "-sl", "Skips matching packages with the language of the system. "
                           "By default unpacked resources packages will match the system languages." },
            Option{ "-sp", "Skips matching packages with of the same system. "
                           "By default unpacked application packages will only match the platform." },
            Option{ "-extract-all", "Extracts all packages from the bundle." },
            Option{ "-pfn-flat", "Unpacks bundle's files to a subdirectory under the specified output path, "
                                 "named after the package full name. Unpacks packages to subdirectories also "
                                 "under the specified output path, named after the package full name. "
                                 "By default unpacked packages will be nested inside the bundle folder." },
            Option{ TOOL_HELP_COMMAND_STRING, "Displays this help text." },
        }
    };

    result.SetDescription({
        "Extracts files and packages within the bundle at the input <bundle> name to",
        "the specified output <directory>. By default, the bundle files will be in",
        "<directory>, while the applicable packages will be placed in subdirectories",
        "named for their package full name. See the available flags to control which",
        "packages are unpacked, and where they are output.",
        });

    result.SetInvocationFunc([](const Invocation& invocation)
        {
            return UnpackBundle(
                GetPackUnpackOptionForBundle(invocation),
                GetValidationOption(invocation),
                GetApplicabilityOption(invocation),
                const_cast<char*>(invocation.GetOptionValue("-p").c_str()),
                const_cast<char*>(invocation.GetOptionValue("-d").c_str()));
        });

    return result;
}

#ifdef MSIX_PACK
Command CreatePackCommand()
{
    Command result{ "pack", "Pack files from disk to a package",
        {
            Option{ "-d", "Input directory path.", true, 1, "directory" },
            Option{ "-p", "Output package file path.", true, 1, "package" },
            Option{ TOOL_HELP_COMMAND_STRING, "Displays this help text." },
        }
    };

    result.SetDescription({
        "Creates an app package at <package> by adding all the files from the",
        "specified input <directory>. You must include a valid package manifest",
        "file named AppxManifest.xml in the directory provided.",
        });

    result.SetInvocationFunc([](const Invocation& invocation)
        {
            return PackPackage(
                MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE,
                MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL,
                const_cast<char*>(invocation.GetOptionValue("-d").c_str()),
                const_cast<char*>(invocation.GetOptionValue("-p").c_str()));
        });

    return result;
}

Command CreateBundleCommand()
{
    Command result{ "bundle", "Create a new app bundle from files on disk",
        {
            Option{ "-d", "Input directory path.", false, 1, "inputDirectory" },
            Option{ "-p", "Output bundle file path.", true, 1, "outputBundle" },
            Option{ "-f", "Mapping file path.", false, 1, "mappingFile" },
            Option{ "-bv", "Specifies the version number of the bundle being created. The version "
                            "must be in dotted - quad notation of four integers "
                            "<Major>.<Minor>.<Build>.<Revision> ranging from 0 to 65535 each. If the "
                            "-bv option is not specified or is set to 0.0.0.0, the bundle is created "
                            "using the current date - time formatted as the version: "
                            "<Year>.<Month - Day>.<Hour - Minute>.<Second - Millisecond>.", false, 1, "version" },
            Option{ "-mo", "Generates a bundle manifest only, instead of a full bundle. Input files must all " 
                            "be package manifests in XML format if this option is specified." },
            Option{ "-fb", "Generates a fully sparse bundle where all packages are references to "
                           "packages that exist outside of the bundle file." },
            Option{ "-o", "Forces the output to overwrite any existing files with the "
                           "same name.By default, the user is asked whether to overwrite existing "
                           "files with the same name.You can't use this option with -no." },
            Option{ "-no","Prevents the output from overwriting any existing files "
                           "with the same name. By default, the user is asked whether to overwrite "
                           "existing files with the same name.You can't use this option with -o." },
            Option{ "-v", "Enables verbose output of messages to the console."},
            Option{ TOOL_HELP_COMMAND_STRING, "Displays this help text." },
        }
    };

    result.SetDescription({
        "Creates an app bundle at <output bundle name> by adding all files from",
        "either <content directory> (including subfolders) or a list of files within",
        "<mapping file>. If either source contains a bundle manifest, it will be",
        "ignored.",
        });

    result.SetInvocationFunc([](const Invocation& invocation)
        {
            char* directoryPath = (invocation.IsOptionPresent("-d")) ?
                const_cast<char*>(invocation.GetOptionValue("-d").c_str()) : nullptr;

            char* mappingFile = (invocation.IsOptionPresent("-f")) ?
                const_cast<char*>(invocation.GetOptionValue("-f").c_str()) : nullptr;

            char* version = (invocation.IsOptionPresent("-bv")) ?
                const_cast<char*>(invocation.GetOptionValue("-bv").c_str()) : nullptr;

            return PackBundle(
                GetBundleOptions(invocation),
                directoryPath,
                const_cast<char*>(invocation.GetOptionValue("-p").c_str()),
                mappingFile,
                version);
        });

    return result;
}

#endif

#pragma endregion

// Defines the grammar of commands and each command's associated options,
int main(int argc, char* argv[])
{
    std::cout << "Microsoft (R) makemsix version " << SDK_VERSION << std::endl;
    std::cout << "Copyright (C) 2017 Microsoft.  All rights reserved." << std::endl;

    std::vector<Command> commands = {
        CreateUnpackCommand(),
        CreateUnbundleCommand(),
        #ifdef MSIX_PACK
        CreatePackCommand(),
        CreateBundleCommand(),
        #endif
    };

    // Help command is always last
    commands.emplace_back(CreateHelpCommand(commands));
    const Command& mainHelpCommand = commands.back();

    Invocation invocation;

    if (!invocation.Parse(commands, argc, argv))
    {
        std::cout << std::endl;
        std::cout << "Error: " << invocation.GetErrorText() << std::endl;

        if (invocation.GetParsedCommand())
        {
            invocation.GetParsedCommand()->PrintHelpText(invocation);
        }
        else
        {
            mainHelpCommand.Invoke(invocation);
        }

        return -1;
    }

    int result = invocation.Run();

    if (result != 0)
    {        
        std::cout << "Error: 0x" << std::hex << result << std::endl;
        if (!invocation.GetErrorText().empty())
        {
            std::cout << "Error: " << invocation.GetErrorText() << std::endl;
        }

        Text text;
        auto logResult = MsixGetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {
            std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else 
        {
            std::cout << "UNABLE TO GET LOG WITH HR=0x" << std::hex << logResult << std::endl;
        }
    }
    return result;
}
