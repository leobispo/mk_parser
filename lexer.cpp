#include <fstream>
#include <sstream>
#include <iostream>

#include <string>
#include <deque>
#include <vector>
#include <memory>
#include <unordered_map>

template <typename T>
std::string join(const T &v, const std::string &delim) {
    std::string s;
    for (const auto &i : v) {
        if (&i != &v[0]) {
            s += delim;
        }
        s += i;
    }

    return s;
}

//TODO: Write the .h files!!

class _Exception : public std::exception
{
    public:
        _Exception(const std::string &reason);

        const char *what() const throw();
    private:
        std::string _reason;
};

_Exception::_Exception(const std::string &reason) : _reason(reason)
{
}

const char *_Exception::what() const throw()
{
    return _reason.c_str();
}

#define Exception(value) _Exception("LINE=" + std::to_string(__LINE__) + " " \
                                    + value)

class Token
{
    public:
        enum class Type { DOLLAR, OPEN_PARENTHESIS, CLOSE_PARENTHESIS, IFEQ,
                          ENDIF, COMMA, ASSIGN, COLLON, ALPHANUM, BACKSLASH,
                          SLASH, SPACE, NUMBER, NEW_LINE, INVALID, CONCAT,
                          END
                        };

        Token();
        Token(Type type, const std::string &value);

        const std::string &value() const;
        Type type() const;
    private:
        Type        _type;
        std::string _value;
};

Token::Token() : _type(Type::INVALID)
{
}

Token::Token(Type type, const std::string &value) : _type(type), _value(value)
{
}

const std::string &Token::value() const
{
    return _value;
}

Token::Type Token::type() const
{
    return _type;
}

class ExtAST
{
    public:
        virtual ~ExtAST();

        virtual std::string codeGen() const = 0;
};

ExtAST::~ExtAST()
{
}

class AttributeAST : public ExtAST
{
    public:
        enum class Type { ASSIGN, CONCAT };

        AttributeAST(Type type, const std::string &key,
                     const std::vector<std::string> &value)
            : _type(type), _key(key), _value(value)
        {
        }

        Type type() const;
        const std::string &key() const;
        const std::vector<std::string> &value() const;

        void appendValues(const std::vector<std::string> &values);
    private:
        Type _type;
        std::string _key;
        std::vector<std::string> _value;

        std::string codeGen() const;
};

AttributeAST::Type AttributeAST::type() const
{
    return _type;
}

const std::string &AttributeAST::key() const
{
    return _key;
}

const std::vector<std::string> &AttributeAST::value() const
{
    return _value;
}

void AttributeAST::appendValues(const std::vector<std::string> &values)
{
    _value.insert(_value.end(), values.begin(), values.end());
}

std::string AttributeAST::codeGen() const
{
    return "";
}

class BlockAST : public ExtAST
{
    public:
        static
        std::unordered_map<std::string, std::pair<std::string, std::string>>
        _libraryMap;

        typedef
        std::unordered_map<std::string,
                           std::shared_ptr<AttributeAST>> Attributes;

        typedef
        std::function<std::shared_ptr<ExtAST>(
                      const Attributes &, const std::string &,
                      std::deque<Token> *)> ParseFunction;

        BlockAST(const std::string &file);

        BlockAST(const std::string &file,
                 const std::shared_ptr<Attributes> &attributes,
                 const std::string &waitingBlock);

        virtual ~BlockAST();

        void parse(std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseProgram(const Attributes &attributes, const std::string &file,
                     std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseLibrary(const Attributes &attributes, const std::string &file,
                     std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseNodeJsAddon(const Attributes &attributes, const std::string &file,
                         std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseNodeJsTest(const Attributes &attributes, const std::string &file,
                        std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseTest(const Attributes &attributes, const std::string &file,
                  std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseSubMake(const Attributes &attributes, const std::string &file,
                     std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseSubMakes(const Attributes &attributes, const std::string &file,
                      std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseVOWSCoffeeTest(const Attributes &attributes,
                            const std::string &file,
                            std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parsePythonProgram(const Attributes &attributes,
                           const std::string &file,
                           std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseVOWSJsTest(const Attributes &attributes, const std::string &file,
                        std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseCompileOption(const Attributes &attributes,
                           const std::string &file,
                            std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parseAddSources(const Attributes &attributes, const std::string &file,
                        std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parsePythonModule(const Attributes &attributes,
                          const std::string &file,
                          std::deque<Token> *tokens);

        static std::shared_ptr<ExtAST>
        parsePythonTest(const Attributes &attributes, const std::string &file,
                        std::deque<Token> *tokens);

        std::string codeGen() const;
    private:
        std::string                                           _waitingBlock;
        std::string                                           _file;
        std::shared_ptr<Attributes>                           _attributes;
        static std::unordered_map<std::string, ParseFunction> _functions;
        std::vector<std::shared_ptr<ExtAST>>                  _AST;

        std::shared_ptr<ExtAST>
        parseIfeq(std::deque<Token> *tokens) const;

        std::shared_ptr<AttributeAST>
        parseAttribute(std::deque<Token> *tokens) const;

        static std::string
        expandAttribute(const Attributes &attributes,
                        std::vector<std::string> *result,
                        std::deque<Token> *tokens,
                        const std::string &file);

        static std::vector<std::string>
        parseValues(const Attributes &attributes, const std::string &file,
                    std::deque<Token> *tokens);

        static std::vector<std::vector<std::string>>
        parseFunctionArgs(const Attributes &attributes,
                          const std::string &file,
                          std::deque<Token> *tokens,
                          size_t number);

        std::shared_ptr<ExtAST> parseFunction(std::deque<Token> *tokens) const;
};

std::unordered_map<std::string, BlockAST::ParseFunction>
BlockAST::_functions =
{
    { "program"                  , &BlockAST::parseProgram        },
    { "library"                  , &BlockAST::parseLibrary        },
    { "nodejs_addon"             , &BlockAST::parseNodeJsAddon    },
    { "nodejs_module"            , &BlockAST::parseNodeJsAddon    }, //TODO: IS THIS CORRECT?
    { "nodejs_test"              , &BlockAST::parseNodeJsTest     },
    { "test"                     , &BlockAST::parseTest           },
    { "include_sub_make"         , &BlockAST::parseSubMake        },
    { "include_sub_makes"        , &BlockAST::parseSubMakes       },
    { "vowscoffee_test"          , &BlockAST::parseVOWSCoffeeTest },
    { "python_program"           , &BlockAST::parsePythonProgram  },
    { "vowsjs_test"              , &BlockAST::parseVOWSJsTest     },
    { "set_compile_option"       , &BlockAST::parseCompileOption  },
    { "set_single_compile_option", &BlockAST::parseCompileOption  },
    { "add_sources"              , &BlockAST::parseAddSources     },
    { "python_module"            , &BlockAST::parsePythonModule   },
    { "python_test"              , &BlockAST::parsePythonTest     }
};

std::unordered_map<std::string, std::pair<std::string, std::string>>
BlockAST::_libraryMap = {
    { "ACE"                  , {"$(ACE_LIB)", "$(ACE_FLAGS)"} },
    { "boost_thread"         , { "$(BOOST_THREAD_LIB)", "" }         },
    { "boost_regex"          , { "$(BOOST_REGEX_LIB)", "" }           },
    { "zeromq"               , { "$(ZMQ_LIB)", "" }                   },
    { "lzma"                 , { "$(LZMA_LIB)", "" }                  },
    { "boost_filesystem"     , { "$(BOOST_FILESYSTEM_LIB)", "" }      },
    { "mongoclient"          , { "$(MONGODB_LIB)", "" }               },
    { "boost_program_options", { "$(BOOST_PROGRAM_OPTIONS_LIB)", "" } },
    { "lapack"               , { "$(LAPACK_LIBS)", "" }               },
    { "blas"                 , { "$(BLAS_LIBS)", "" }                 },
    { "fortran"              , { "$(FLIBS)", "" }                     },
    { "zmq"                  , { "$(ZMQ_LIB)", "" }                   },
    { "zookeeper_mt"         , { "$(ZOOKEEPER_LIB)", "" }             },
    { "curl"                 , { "$(LIBCURL)" , "" }                  },
    { "curlpp"               , { "$(CURLPP)", "" }                    },
    { "ssh2"                 , { "$(SSH2_LIB)", "" }                  },
    { "hiredis"              , { "$(HIREDIS_LIB)", "" }               },
    { "crypto++"             , { "$(CRYPTO_LIB)", "" }                }
};

BlockAST::BlockAST(const std::string &file) : _file(file),
    _attributes(new Attributes())
{
    (*_attributes)["PYTHON_ENABLED"] =
    std::shared_ptr<AttributeAST>(new AttributeAST(AttributeAST::Type::ASSIGN,
                                  "PYTHON_ENABLED", { "0" }));

    (*_attributes)["BOOST_VERSION"] =
    std::shared_ptr<AttributeAST>(new AttributeAST(AttributeAST::Type::ASSIGN,
                                  "BOOST_VERSION", { "52" }));
}

BlockAST::BlockAST(const std::string &file,
                   const std::shared_ptr<Attributes> &attributes,
                   const std::string &waitingBlock)
    : _file(file), _attributes(attributes), _waitingBlock(waitingBlock)
{
}

BlockAST::~BlockAST()
{
}

void BlockAST::parse(std::deque<Token> *tokens)
{
    while (!tokens->empty()) {
        switch (tokens->front().type()) {
            case Token::Type::NEW_LINE:
            case Token::Type::SPACE:
                tokens->pop_front();
            break;
            case Token::Type::ALPHANUM:
                if (tokens->front().value() == "ifeq") {
                    _AST.push_back(parseIfeq(tokens));
                }
                else if (tokens->front().value() == "endif") {
                    if (_waitingBlock != "ifeq")
                        throw new Exception("Not expecting an endif at this"
                                            " point");

                    tokens->pop_front();
                    return;
                }
                else {
                    auto attr = parseAttribute(tokens);
                    if (attr) {
                        if (attr->type() == AttributeAST::Type::ASSIGN) {
                            (*_attributes)[attr->key()] = attr;
                        }
                        else {
                            auto t = _attributes->find(attr->key());
                            if (t == _attributes->end())
                                throw Exception("Can't find attribute "
                                                + attr->key());
                            else {
                                auto values = attr->value();
                                t->second->appendValues(attr->value());
                            }
                        }
                    }
                }

            break;
            case Token::Type::DOLLAR:
            {
                tokens->pop_front();
                auto function = parseFunction(tokens);
                if (function)
                    _AST.push_back(function);
            }
            break;
            case Token::Type::END:
                tokens->pop_front();
                if (!_waitingBlock.empty())
                    throw new Exception("Expecting an end of block for: "
                                        + _waitingBlock);

            break;
            default:
                throw Exception("Not expecting token : "
                                + tokens->front().value());
        }
    }
}

std::string BlockAST::codeGen() const
{
    std::string code;
    for (const auto &element : _AST)
        code += element->codeGen();

    return code;
}

class SubMakesAST : public ExtAST
{
    public:
        SubMakesAST(const std::vector<std::string> &subDirs,
                    const std::string &dir)
            : _subDirs(subDirs), _dir(dir)
        {
        }

        std::string codeGen() const;

    private:
        std::vector<std::string> _subDirs;
        std::string              _dir;
};

class MKParser
{
    public:
        MKParser(const std::string &file,
                 const std::vector<std::string> &subdirs = {});

        void run(std::string output = "");
    private:
        SubMakesAST  _subMakes;
        BlockAST     _root;
        mutable char _lastChar = 0;
        std::string  _file;

        std::deque<Token> lexer() const;
        Token nextToken(std::ifstream &is) const;

        std::string codeGen() const;
};

MKParser::MKParser(const std::string &file,
                   const std::vector<std::string> &subdirs)
    : _subMakes(subdirs, file.substr(0, file.find_last_of("/")) + "/"),
      _root(file), _file(file)
{
}

void MKParser::run(std::string output)
{
    std::deque<Token> tokens = lexer();

    if (output.empty())
        output = _file.substr(0, _file.find_last_of("/")) + "/Makefile.am";

    _root.parse(&tokens);

    std::ofstream out(output);
    out << codeGen();
    out.close();
}

std::deque<Token> MKParser::lexer() const
{
    std::deque<Token> tokens;
    _lastChar = 0;

    std::ifstream is(_file);
    while (is.good()) {
        auto token = nextToken(is);
        tokens.push_back(token);
    }

    is.close();

    return tokens;
}

Token MKParser::nextToken(std::ifstream &is) const
{
    auto c = _lastChar ? _lastChar : is.get();

    _lastChar = 0;
    if (c == '#') {
        do c = is.get();
        while (c != EOF && c != '\n' && c != '\r');
    }

    if (c == EOF)
        return Token(Token::Type::END, "END");
    if (c == ',')
        return Token(Token::Type::COMMA, ",");
    if (c == '$')
        return Token(Token::Type::DOLLAR, "$");
    if (c == '(')
        return Token(Token::Type::OPEN_PARENTHESIS, "(");
    if (c == ')')
        return Token(Token::Type::CLOSE_PARENTHESIS, ")");
    if (c == '/')
        return Token(Token::Type::SLASH, R"(/)");
    if (c == '\\')
        return Token(Token::Type::BACKSLASH, R"(\)");
    if (c == '\r' || c == '\n')
        return Token(Token::Type::NEW_LINE, R"(\n)");
    if (c == ' ' || c == '\t') {
        while (c == ' ' || c == '\t')
            c = is.get();

        _lastChar = c;
        return Token(Token::Type::SPACE, "SPACE");
    }

    if (isdigit(c)) {
        std::string num;
        do {
            num.push_back(c);
            c = is.get();
        } while (isdigit(c));

        _lastChar = c;
        return Token(Token::Type::NUMBER, num);
    }

    if (isalpha(c) || c == '_' || c == '-' || c == '.') {
        std::string alphanum;
        bool isProtected = false;
        do {
            alphanum.push_back(c);
            c = is.get();
            if (c == '\"') isProtected = !isProtected;

        } while (isalnum(c) || c == '_' || c == '.' || c == '+' || c == '-' ||
                c == '=' || c == '/' || c == '\"' || c == '\\' ||
                (isProtected && (c == '$' || c == '(' || c == ')')));

        _lastChar = c;
        return Token(Token::Type::ALPHANUM, alphanum);
    }

    if (c == '+') {
        c = is.get();
        if (c == '=')
            return Token(Token::Type::CONCAT, "+=");
    }

    if (c == '=')
        return Token(Token::Type::ASSIGN, "=");

    if (c == ':') {
        c = is.get();
        if (c == '=')
            return Token(Token::Type::ASSIGN, ":=");

        _lastChar = c;
        return Token(Token::Type::COLLON, ":");
    }

    std::string err("Invalid Token: " + _file + " ");
    err += c;
    throw Exception(err);
}

std::string MKParser::codeGen() const
{
    std::string code;
    code +=  "ACLOCAL_AMFLAGS = -I m4\n\n";
    code +=  "AM_CPPFLAGS = \\\n";
    code +=  "  -I $(abs_top_builddir)\n\n";

    code +=  "lib_LTLIBRARIES =\n";
    code +=  "noinst_LTLIBRARIES =\n\n";

    code += "NODEJS_LIBTOOL_FLAGS = \\\n";
    code += "-shrext .node \\\n";
    code += "-module \\\n";
    code += "-shared \\\n";
    code += "-avoid-version \\\n";
    code += "-rpath $(abs_builddir) \\\n";
    code += "-fPIC \\\n";
    code += "-Wall \\\n";
    code += "-m64 \\\n";
    code += "-fdata-sections \\\n";
    code += "-ffunction-sections \\\n";
    code += "-fno-strict-aliasing \\\n";
    code += "-fno-rtti \\\n";
    code += "-fno-exceptions\n\n";
    code += "TESTS =\n";
    code += "check_PROGRAMS =\n";
    code += "bin_PROGRAMS =\n";
    code += "SUBDIRS =\n\n";

    code += _subMakes.codeGen();
    code += _root.codeGen();

    code += "\nTESTS_ENVIRONMENT = $(abs_top_builddir)/test_driver.sh "
        "NODE=$(NODEJS) VOWS=$(VOWS) NODE_LIBS=\""
        "$(noinst_LTLIBRARIES)\"\n";

    code += "TESTS += $(abs_top_builddir)/runjstest.sh\n\n";

    code += "node_prefix=$(exec_prefix)/node_modules\n\n";
    code += "install-exec-hook:\n";
    code += "\tmkdir -p $(node_prefix)\n";
    code += "\tcp -rf .libs/*.node $(node_prefix)\n";
    code += "uninstall-hook:\n";

    code += "\tfor i in $(noinst_LTLIBRARIES); do lib=`echo \"$$i\" | "
        "sed 's/\\.la/\\.node/g'`; rm $(node_prefix)/$$lib; done\n";

    code += "\tif find \"$(node_prefix)\" -maxdepth 0 -empty | read; "
        "then rm -rf $(node_prefix); fi\n";

    return code;
}

class ProgramAST : public ExtAST
{
    public:
        ProgramAST(const std::string &name,
                  const std::vector<std::string> &dependencies,
                  const std::vector<std::string> &sources,
                  const std::vector<std::string> &targets)
            : _name(name), _dependencies(dependencies), _sources(sources),
              _targets(targets)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _sources;
        std::vector<std::string> _targets;

        std::string codeGen() const;
};

std::string ProgramAST::codeGen() const
{
    std::string code;

    code = "bin_PROGRAMS += " + _name + "\n\n";

    code += _name + "_SOURCES = \\\n  ";
    if (_sources.empty())
        code += _name + ".cc";
    else
        code += join(_sources, " \\\n  ");

    if (!_dependencies.empty()) {
        code += "\n\n" + _name + "_LDADD = \\\n  ";

        std::vector<std::string> cxxFlags;
        auto copyDependencies = _dependencies;
        for (auto &dependency : copyDependencies) {
            auto t = BlockAST::_libraryMap.find(dependency);
            if (t != BlockAST::_libraryMap.end()) {
                dependency = t->second.first;
                if (!t->second.second.empty())
                    cxxFlags.push_back(t->second.second);
            }
        }
        code += join(copyDependencies, " \\\n  ");
        code += "\n\n";

        if (!cxxFlags.empty()) {
            code += _name + "_CXXFLAGS = \\\n  ";
            code += join(cxxFlags, " \\\n  ");
            code += "\n\n";
        }
    }

    return code;
}

/*
 * # add a program
 * # $(1): name of the program
 * # $(2): libraries to link with
 * # $(3): name of files to include in the program.  If not included or empty,
 * #       $(1).cc assumed
 * # $(4): list of targets to add this program to
 */
std::shared_ptr<ExtAST>
BlockAST::parseProgram(const Attributes &attributes, const std::string &file,
                       std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 4);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    return std::make_shared<ProgramAST>(args.at(0).at(0), args.at(1),
                                        args.at(2), args.at(3));
}

class LibraryAST : public ExtAST
{
    public:
        LibraryAST(const std::string &name,
                   const std::vector<std::string> &sources,
                   const std::vector<std::string> &dependencies,
                   const std::string & output, const std::string &extension,
                   const std::string &buildName)
            : _name(name), _sources(sources), _dependencies(dependencies),
              _extension(extension), _output(output), _buildName(buildName)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _sources;
        std::vector<std::string> _dependencies;
        std::string              _output;
        std::string              _extension;
        std::string              _buildName;

        std::string codeGen() const;
};

std::string LibraryAST::codeGen() const
{
    std::string code;

    auto libName = "lib" + (_output.empty() ? _name : _output);

    code = "lib_LTLIBRARIES += " + libName + ".la\n\n";

    code += libName + "_la_LDFLAGS = -avoid-version\n";

    code += libName + "_la_SOURCES = \\\n  ";
    code += join(_sources, " \\\n  ");

    code += "\n\n";
    if (!_dependencies.empty()) {
        code += libName + "_la_LIBADD = \\\n  ";

        std::vector<std::string> cxxFlags;
        auto copyDependencies = _dependencies;
        for (auto &dependency : copyDependencies) {
            auto t = BlockAST::_libraryMap.find(dependency);
            if (t != BlockAST::_libraryMap.end()) {
                dependency = t->second.first;
                if (!t->second.second.empty())
                    cxxFlags.push_back(t->second.second);
            }
        }

        code += join(copyDependencies, " \\\n  ");
        code += "\n\n";
        if (!cxxFlags.empty()) {
            code += libName + "_la_CXXFLAGS = \\\n  ";
            code += join(cxxFlags, " \\\n  ");
            code += "\n\n";
        }
    }

    return code;
}

/*
 * # $(1): name of the library
 * # $(2): source files to include in the library
 * # $(3): libraries to link with
 * # $(4): output name; default lib$(1)
 * # $(5): output extension; default .so
 * # $(6): build name; default SO
 */
std::shared_ptr<ExtAST>
BlockAST::parseLibrary(const Attributes &attributes, const std::string &file,
                       std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 6);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(1).empty())
        args.at(1).push_back(args.at(0).at(0) + ".cc");

    if (args.at(3).size() > 1)
        throw Exception("Must have maximum 1 output name");

    if (args.at(4).size() > 1)
        throw Exception("Must have maximum 1 output extension");

    if (args.at(5).size() > 1)
        throw Exception("Must have maximum 1 build namer");

    auto output = args.at(3).empty() ? "" : args.at(3).at(0);
    auto extension = args.at(4).empty() ? "" : args.at(4).at(0);
    auto buildName = args.at(5).empty() ? "" : args.at(5).at(0);

    //TODO: CHANGE ME TO CHECK FOR BUILD NAME - And check if the path is
    //      absolute or relative!!
    auto libPath = file.substr(0, file.find_last_of("/")) + "/" + "lib"
                               + args.at(0).at(0) + ".la";

    _libraryMap[args.at(0).at(0)].first = libPath;

    return std::make_shared<LibraryAST>(args.at(0).at(0), args.at(1),
                                        args.at(2), output, extension,
                                        buildName);
}

class NodeJsAddonAST : public ExtAST
{
    public:
        NodeJsAddonAST(const std::string &name,
                       const std::vector<std::string> &sources,
                       const std::vector<std::string> &dependencies,
                       const std::vector<std::string> &otherJs)
            : _name(name), _sources(sources), _dependencies(dependencies),
              _otherJs(otherJs)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _sources;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _otherJs;

        std::string codeGen() const;
};

std::string NodeJsAddonAST::codeGen() const
{
    std::string code;

    auto libName = _name;

    code = "noinst_LTLIBRARIES += " + libName + ".la\n\n";
    code += libName + "_la_LDFLAGS = $(NODEJS_LIBTOOL_FLAGS)\n";
    code += libName + "_la_CXXFLAGS = \n"; //TODO: FIXME!!

    code += libName + "_la_SOURCES = \\\n  ";
    code += join(_sources, " \\\n  ");

    if (!_dependencies.empty())
        code += "\n\n" + libName + "_la_LIBADD = \\\n  ";

    auto copyDependencies = _dependencies;
    for (auto &dependency : copyDependencies) {
        auto t = BlockAST::_libraryMap.find(dependency);
        if (t != BlockAST::_libraryMap.end())
            dependency = t->second.first;
        else
            std::cout << dependency << std::endl;
    }
    code += join(copyDependencies, " \\\n  ");
    code += "\n\n";

    return code;
}

/*
 * # $(1): name of the addon
 * # $(2): source files to include in the addon
 * # $(3): libraries to link with
 * # $(4): other node.js addons that need to be linked in with this one
 */
std::shared_ptr<ExtAST>
BlockAST::parseNodeJsAddon(const Attributes &attributes,
                           const std::string &file,
                           std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 4);

    if (args.at(0).size() != 1)
        throw Exception("Must pass only 1 name");

    if (args.at(1).empty())
        throw Exception("Must pass at least 1 source file");

    return std::make_shared<NodeJsAddonAST>(args.at(0).at(0), args.at(1),
                                            args.at(2), args.at(3));
}

class NodeJsTestAST : public ExtAST
{
    public:
        NodeJsTestAST(const std::string &name,
                      const std::vector<std::string> &dependencies,
                      const std::vector<std::string> &options,
                      const std::string &testName,
                      const std::vector<std::string> &testOptions)
            : _name(name), _dependencies(dependencies), _options(options),
              _testName(testName), _testOptions(testOptions)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _options;
        std::string              _testName;
        std::vector<std::string> _testOptions;

        std::string codeGen() const;
};

std::string NodeJsTestAST::codeGen() const
{
    return "";
}

/*
 * # $(1) name of the test (the javascript file that contains the test case)
 * # $(2) node.js modules on which it depends
 * # $(3) options to the node executable
 * # $(4) test name
 * # $(5) test options
 */
std::shared_ptr<ExtAST>
BlockAST::parseNodeJsTest(const Attributes &attributes,
                          const std::string &file, std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 5);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(3).size() > 1)
        throw Exception("Must have maximum 1 testName");

    auto testName = args.at(3).empty() ? "" : args.at(3).at(0);

    return std::make_shared<NodeJsTestAST>(args.at(0).at(0), args.at(1),
                                           args.at(2), testName, args.at(4));
}

class TestAST : public ExtAST
{
    public:
        TestAST(const std::string &name,
                const std::vector<std::string> &dependencies,
                const std::vector<std::string> &style,
                const std::vector<std::string> &targets)
            : _name(name), _dependencies(dependencies), _style(style),
              _targets(targets)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _style;
        std::vector<std::string> _targets;

        std::string codeGen() const;
};

std::string TestAST::codeGen() const
{
    std::string code;
    code += "TESTS += " + _name + "\n";
    code += "check_PROGRAMS += " + _name + "\n";
    code += _name + "_SOURCES = " + _name + ".cc\n";
    code += _name + "_CXXFLAGS =\n"; //TODO: ADD FLAGS HERE

    if (!_dependencies.empty()) {
        code += _name + "_LADD = \\\n  ";

        auto copyDependencies = _dependencies;
        for (auto &dependency : copyDependencies) {
            auto t = BlockAST::_libraryMap.find(dependency);
            if (t != BlockAST::_libraryMap.end())
                dependency = t->second.first;
            else
                std::cout << dependency << std::endl;
        }
        code += join(copyDependencies, " \\\n  ");
        code += "\n\n";
    }

    return code;
}

/*
 * # add a test case
 * # $(1) name of the test
 * # $(2) libraries to link with
 * # $(3) test style.  boost = boost test framework, and options: manual,
 *                     valgrind
 * # $(4) testing targets to add it to
 */
std::shared_ptr<ExtAST>
BlockAST::parseTest(const Attributes &attributes, const std::string &file,
                    std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file,tokens, 4);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    return std::make_shared<TestAST>(args.at(0).at(0), args.at(1), args.at(2),
                                     args.at(3));
}

class SubMakeAST : public ExtAST
{
    public:
        SubMakeAST(const std::string &name, const std::string &basedir,
                   const std::string &dir, const std::string &makefile)
            : _name(name), _basedir(basedir), _dir(dir), _makefile(makefile)
        {
        }

    private:
        std::string _name;
        std::string _basedir;
        std::string _dir;
        std::string _makefile;

        std::string codeGen() const;
};

std::string SubMakeAST::codeGen() const
{
    auto dir = (_dir.empty() ? _name : _dir);

    std::string makefile;

    if (dir != "testing")
        makefile  = (_makefile.empty() ? (dir + ".mk") : _makefile);
    else {
        makefile  = (_makefile.empty() ? (_name + ".mk") : _makefile);
    }

    auto file = _basedir + dir + "/" + makefile;

    MKParser parser(file);
    parser.run(_basedir + dir + "/Makefile.am");

    return "SUBDIRS += " + dir + "\n";
}

/*
 * # arg 1: name
 * # arg 2: dir (optional, is the same as $(1) if not given)
 * # arg 3: makefile (optional, is $(2)/$(1).mk if not given)
 */
std::shared_ptr<ExtAST>
BlockAST::parseSubMake(const Attributes &attributes, const std::string &file,
                       std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 3);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(1).size() > 1)
        throw Exception("Must have maximum 1 dir");

    if (args.at(2).size() > 1)
        throw Exception("Must have maximum 1 makefile");

    auto dir = args.at(1).empty() ? "" : args.at(1).at(0);
    auto makefile = args.at(2).empty() ? "" : args.at(2).at(0);

    return std::make_shared<SubMakeAST>(args.at(0).at(0), file.substr(0,
                                        file.find_last_of("/")) + "/",
                                        dir, makefile);
}

std::string SubMakesAST::codeGen() const
{
    std::string code;
    for (auto &subDir : _subDirs) {
        SubMakeAST ast(subDir, _dir, "", "");
        code += static_cast<ExtAST *>(&ast)->codeGen();
    }

    return code;
}

/*
 * # arg 1: names
 */
std::shared_ptr<ExtAST>
BlockAST::parseSubMakes(const Attributes &attributes,
                        const std::string &file,
                        std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 1);
    return std::make_shared<SubMakesAST>(args.at(0), file.substr(0,
                                         file.find_last_of("/")) + "/");
}

class VOWSCoffeeTestAST : public ExtAST
{
    public:
        VOWSCoffeeTestAST(const std::string &name,
                          const std::vector<std::string> &dependencies,
                          const std::vector<std::string> &options,
                          const std::string &target,
                          const std::vector<std::string> &testOptions)
            : _name(name), _dependencies(dependencies), _options(options),
              _target(target), _testOptions(testOptions)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _options;
        std::string              _target;
        std::vector<std::string> _testOptions;

        std::string codeGen() const;
};

std::string VOWSCoffeeTestAST::codeGen() const
{
    return "";
}

/**
 * # $(1) name of the test (the javascript file that contains the test case)
 * # $(2) node.js modules on which it depends
 * # $(3) options to the vows executable
 * # $(4) test target
 * # $(5) test options (eg, manual)
 */
std::shared_ptr<ExtAST>
BlockAST::parseVOWSCoffeeTest(const Attributes &attributes,
                              const std::string &file,
                              std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 5);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(3).size() > 1)
        throw Exception("Must have maximum 1 testName");

    auto testName = args.at(3).empty() ? "" : args.at(3).at(0);

    return std::make_shared<VOWSCoffeeTestAST>(args.at(0).at(0), args.at(1),
                                               args.at(2), testName,
                                               args.at(4));
}

class PythonProgramAST : public ExtAST
{
    public:
        PythonProgramAST(const std::string &name,
                         const std::vector<std::string> &sources,
                          const std::vector<std::string> &dependencies)
            : _name(name), _sources(sources), _dependencies(dependencies)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _sources;
        std::vector<std::string> _dependencies;

        std::string codeGen() const;
};

std::string PythonProgramAST::codeGen() const
{
    return "";
}

/*
 * # $(1): name of python program
 * # $(2): python source file to copy
 * # $(3): python modules it depends upon
 */
std::shared_ptr<ExtAST>
BlockAST::parsePythonProgram(const Attributes &attributes,
                             const std::string &file,
                             std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 3);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    return std::make_shared<PythonProgramAST>(args.at(0).at(0), args.at(1),
                                              args.at(2));
}

class VOWSJsTestAST : public ExtAST
{
    public:
        VOWSJsTestAST(const std::string &name,
                          const std::vector<std::string> &dependencies,
                          const std::vector<std::string> &options,
                          const std::string &target,
                          const std::vector<std::string> &testOptions)
            : _name(name), _dependencies(dependencies), _options(options),
              _target(target), _testOptions(testOptions)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _options;
        std::string              _target;
        std::vector<std::string> _testOptions;

        std::string codeGen() const;
};

std::string VOWSJsTestAST::codeGen() const
{
    return "";
}

/*
 * # $(1) name of the test (the javascript file that contains the test case)
 * # $(2) node.js modules on which it depends
 * # $(3) options to the vows executable
 * # $(4) test target
 * # $(5) test options (eg, manual)
 */
std::shared_ptr<ExtAST>
BlockAST::parseVOWSJsTest(const Attributes &attributes,
                          const std::string &file,
                          std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 5);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(3).size() > 1)
        throw Exception("Must have maximum 1 testName");

    auto testName = args.at(3).empty() ? "" : args.at(3).at(0);

    return std::make_shared<VOWSJsTestAST>(args.at(0).at(0), args.at(1),
                                           args.at(2), testName,
                                           args.at(4));
}

class CompileOptionAST : public ExtAST
{
    public:
        CompileOptionAST(const std::vector<std::string> &fileNames,
                          const std::vector<std::string> &options)
            : _fileNames(fileNames), _options(options)
        {
        }

    private:
        std::vector<std::string> _fileNames;
        std::vector<std::string> _options;

        std::string codeGen() const;
};

std::string CompileOptionAST::codeGen() const
{
    return "";
}

/*
 * # set compile options for a given list of source files
 * # $(1): list of filenames
 * # $(2): compile option
 */
std::shared_ptr<ExtAST>
BlockAST::parseCompileOption(const Attributes &attributes,
                             const std::string &file,
                             std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 2);
    return std::make_shared<CompileOptionAST>(args.at(0), args.at(1));
}

class AddSourcesAST : public ExtAST
{
    public:
        AddSourcesAST(const std::vector<std::string> &fileNames)
            : _fileNames(fileNames)
        {
        }

    private:
        std::vector<std::string> _fileNames;

        std::string codeGen() const;
};

std::string AddSourcesAST::codeGen() const
{
    return "";
}

/*
 * # add a list of source files
 * # $(1): list of filenames
 */
std::shared_ptr<ExtAST>
BlockAST::parseAddSources(const Attributes &attributes,
                          const std::string &file,
                          std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 1);
    return std::make_shared<AddSourcesAST>(args.at(0));
}


class PythonModuleAST : public ExtAST
{
    public:
        PythonModuleAST(const std::string &name,
                         const std::vector<std::string> &sources,
                          const std::vector<std::string> &dependencies,
                          const std::vector<std::string> &libraries)
            : _name(name), _sources(sources), _dependencies(dependencies),
              _libraries(libraries)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _sources;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _libraries;

        std::string codeGen() const;
};

std::string PythonModuleAST::codeGen() const
{
    return "";
}

/*
 * # $(1): name of python module
 * # $(2): list of python source files to copy
 * # $(3): python modules it depends upon
 * # $(4): libraries it depends upon
 */
std::shared_ptr<ExtAST>
BlockAST::parsePythonModule(const Attributes &attributes,
                             const std::string &file,
                             std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 4);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    return std::make_shared<PythonModuleAST>(args.at(0).at(0), args.at(1),
                                             args.at(2), args.at(3));
}

class PythonTestAST : public ExtAST
{
    public:
        PythonTestAST(const std::string &name,
                         const std::vector<std::string> &sources,
                          const std::vector<std::string> &dependencies,
                          const std::vector<std::string> &targets)
            : _name(name), _sources(sources), _dependencies(dependencies),
              _targets(targets)
        {
        }

    private:
        std::string              _name;
        std::vector<std::string> _sources;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _targets;

        std::string codeGen() const;
};

std::string PythonTestAST::codeGen() const
{
    return "";
}

/*
 * # $(1) name of the test
 * # $(2) python modules on which it depends
 * # $(3) test options (e.g. manual)
 * # $(4) test targets
 */
std::shared_ptr<ExtAST>
BlockAST::parsePythonTest(const Attributes &attributes,
                             const std::string &file,
                             std::deque<Token> *tokens)
{
    auto args = parseFunctionArgs(attributes, file, tokens, 4);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    return std::make_shared<PythonTestAST>(args.at(0).at(0), args.at(1),
                                             args.at(2), args.at(3));
}

class IfeqAST : public ExtAST
{
    public:
        IfeqAST(const std::string &check, bool isCheckAttribute,
                const std::string &expected, bool isExpectedAttribute,
                const std::string &file,
                const std::shared_ptr<BlockAST::Attributes> &attributes)
            : _check(check), _isCheckAttribute(isCheckAttribute),
              _expected(expected), _isExpectedAttribute(isExpectedAttribute),
              _root(file, attributes, "ifeq")
        {
        }

        void parse(std::deque<Token> *tokens);

    private:
        static std::unordered_map<std::string, std::string> _ifSubstitute;

        std::string _check;
        bool        _isCheckAttribute;
        std::string _expected;
        bool        _isExpectedAttribute;
        BlockAST    _root;

        std::string codeGen() const;
};

std::unordered_map<std::string, std::string> IfeqAST::_ifSubstitute =
{
    { "CUDA_ENABLED", "HAVE_CUDA" },
    { "CAL_ENABLED" , "HAVE_CAL"  }
};

void IfeqAST::parse(std::deque<Token> *tokens)
{
    _root.parse(tokens);
}

std::string IfeqAST::codeGen() const
{
    std::string code;

    if (_isCheckAttribute) {
        auto it = _ifSubstitute.find(_check);
        if (it != _ifSubstitute.end())
            code += "if " + it->second + "\n";

        code += _root.codeGen();
        code += "endif\n\n";
    }
    else if (!_isExpectedAttribute) {
        if (_check == _expected)
            code += _root.codeGen() + "\n";
    }

    return code;
}

std::shared_ptr<ExtAST>
BlockAST::parseIfeq(std::deque<Token> *tokens) const
{
    tokens->pop_front();
    if (tokens->front().type() == Token::Type::SPACE)
        tokens->pop_front();

    if (tokens->front().type() != Token::Type::OPEN_PARENTHESIS)
        throw Exception("Invalid token, expecting ( after ifeq but got: "
                        + tokens->front().value());

    tokens->pop_front();
    if (tokens->front().type() == Token::Type::SPACE)
        tokens->pop_front();

    std::string check;
    bool isCheckAttribute = false;
    if (tokens->front().type() == Token::Type::DOLLAR) {
        std::vector<std::string> values;
        auto var = expandAttribute(*_attributes, &values, tokens, _file);
        if (!var.empty()) {
            isCheckAttribute = true;
            check = var;
        }
        else if (values.size() == 1)
            check = values.at(0);
        else
            throw Exception("Excpecting 1 value to be checked");
    }
    else if (tokens->front().type() == Token::Type::NUMBER ||
             tokens->front().type() == Token::Type::ALPHANUM) {
        check = tokens->front().value();
        tokens->pop_front();
    }
    else
        throw Exception("Invalid token to check in an ifeq statement");

    if (tokens->front().type() == Token::Type::SPACE)
        tokens->pop_front();

    if (tokens->front().type() != Token::Type::COMMA)
        throw Exception("Expecting a comma on the ifeq statement");

    tokens->pop_front();

    if (tokens->front().type() == Token::Type::SPACE)
        tokens->pop_front();

    std::string expected;
    bool isExpectedAttribute = false;
    if (tokens->front().type() == Token::Type::DOLLAR) {
        std::vector<std::string> values;
        auto var = expandAttribute(*_attributes, &values, tokens, _file);
        if (!var.empty()) {
            isExpectedAttribute = true;
            expected = var;
        }
        else if (values.size() == 1)
            expected = values.at(0);
        else
            throw Exception("Expecting 1 value to be expected");
    }
    else if (tokens->front().type() == Token::Type::NUMBER ||
             tokens->front().type() == Token::Type::ALPHANUM) {
        expected = tokens->front().value();
        tokens->pop_front();
    }
    else
        throw Exception("Invalid token to expect in an ifeq statement");

    if (tokens->front().type() == Token::Type::SPACE)
        tokens->pop_front();

    if (tokens->front().type() != Token::Type::CLOSE_PARENTHESIS)
        throw Exception("Invalid token, expecting ) but got: "
                        + tokens->front().value());

    tokens->pop_front();

    auto ifeq = std::make_shared<IfeqAST>(check, isCheckAttribute,
                                          expected, isExpectedAttribute,
                                          _file, _attributes);
    ifeq->parse(tokens);
    return ifeq;
}

std::shared_ptr<AttributeAST>
BlockAST::parseAttribute(std::deque<Token> *tokens) const
{
    auto attr = tokens->front().value();
    tokens->pop_front();

    if (tokens->front().type() == Token::Type::END)
        throw Exception(R"(Not expecting End of File)");

    if (tokens->front().type() == Token::Type::SPACE)
        tokens->pop_front();

    if (tokens->front().type() == Token::Type::COLLON) {
        do tokens->pop_front();
        while (tokens->front().type() != Token::Type::NEW_LINE &&
               tokens->front().type() != Token::Type::END);

        return nullptr;
    }

    if (tokens->front().type() == Token::Type::CONCAT) {
        tokens->pop_front();
        auto values = parseValues(*_attributes, _file, tokens);
        return std::make_shared<AttributeAST>(AttributeAST::Type::CONCAT,
                                              attr, values);
    }

    if (tokens->front().type() == Token::Type::ASSIGN) {
        tokens->pop_front();
        auto values = parseValues(*_attributes, _file, tokens);
        return std::make_shared<AttributeAST>(AttributeAST::Type::ASSIGN,
                                              attr, values);
    }

    throw Exception(R"(Invalid token, expecting := or += but got: )"
                    + tokens->front().value());
}

std::string
BlockAST::expandAttribute(const Attributes &attributes,
                          std::vector<std::string> *result,
                          std::deque<Token> *tokens,
                          const std::string &file)
{
    if (tokens->front().type() != Token::Type::DOLLAR)
        throw Exception("Invalid token, expecting $ but got: "
                        + tokens->front().value());
    tokens->pop_front();
    if (tokens->front().type() != Token::Type::OPEN_PARENTHESIS)
        throw Exception("Invalid token, expecting ( after $ but got: "
                        + tokens->front().value());

    tokens->pop_front();
    if (tokens->front().type() != Token::Type::ALPHANUM)
        throw Exception("Invalid token, expecting VARIABLE after "
                        "'$(' but got: " + tokens->front().value());

    auto variable = tokens->front().value();

    if (variable == "shell") {
        uint32_t count = 1;
        while (!tokens->empty()) {
            if (tokens->front().type() == Token::Type::NEW_LINE) {
                if (count)
                    throw Exception("Invalid number of (");

                tokens->pop_front();
                return "";
            }
            if (tokens->front().type() == Token::Type::OPEN_PARENTHESIS)
                ++count;
            else if (tokens->front().type() == Token::Type::CLOSE_PARENTHESIS)
                --count;

            tokens->pop_front();
        }

        throw Exception("Expecting token )");
    }

    tokens->pop_front();
    if (tokens->front().type() != Token::Type::CLOSE_PARENTHESIS)
        throw Exception("Invalid token, expecting ) but got: "
                        + tokens->front().value() + " " + file);

    tokens->pop_front();
    const auto v = attributes.find(variable);
    if (v != attributes.end()) {
        *result = v->second->value();
        return "";
    }

    return variable;
}

std::vector<std::string>
BlockAST::parseValues(const Attributes &attributes, const std::string &file,
                      std::deque<Token> *tokens)
{
    if (tokens->front().type() == Token::Type::END)
        throw Exception(R"(Not expecting End of File)");

    std::vector<std::string> values;
    do {
        switch (tokens->front().type()) {
            case Token::Type::SPACE:
                break;
            case Token::Type::NUMBER:
            case Token::Type::ALPHANUM:
                values.push_back(tokens->front().value());
            break;
            case Token::Type::BACKSLASH:
            {
                tokens->pop_front();
                if (tokens->front().type() == Token::Type::SPACE)
                    tokens->pop_front();

                if (tokens->front().type() == Token::Type::NEW_LINE)
                    break;

                throw Exception(R"(Expecting a new line after '\')");
            }
            break;
            case Token::Type::COMMA:
            case Token::Type::NEW_LINE:
            case Token::Type::END:
            case Token::Type::CLOSE_PARENTHESIS:
                return values;
            case Token::Type::DOLLAR:
            {
                expandAttribute(attributes, &values, tokens, file);
                continue;
            }
            break;
            default:
                throw Exception(R"(Invalid token, not expecting: )"
                                + tokens->front().value());
        }
        tokens->pop_front();
    } while (!tokens->empty());

    return values;
}

std::vector<std::vector<std::string>>
BlockAST::parseFunctionArgs(const Attributes &attributes,
                            const std::string &file,
                            std::deque<Token> *tokens,
                            size_t number)
{
    std::vector<std::vector<std::string>> values(number);

    for (auto &value : values) {
        if (tokens->front().type() == Token::Type::SPACE)
            tokens->pop_front();

        if (tokens->front().type() == Token::Type::CLOSE_PARENTHESIS)
            break;

        if (tokens->front().type() != Token::Type::COMMA)
            throw Exception(R"(Invalid token, expecting ',' but got: )"
                            + tokens->front().value());

        tokens->pop_front();
        if (tokens->front().type() == Token::Type::SPACE)
            tokens->pop_front();

        value = parseValues(attributes, file, tokens);
    }

    return values;
}

std::shared_ptr<ExtAST>
BlockAST::parseFunction(std::deque<Token> *tokens) const
{
    std::shared_ptr<ExtAST> ret;

    Token token;

    token = tokens->front();
    tokens->pop_front();
    if (token.type() != Token::Type::OPEN_PARENTHESIS)
        throw Exception("Invalid token, expecting ( after $ but got: "
                        + tokens->front().value());

    token = tokens->front();
    tokens->pop_front();
    if (token.type() == Token::Type::ALPHANUM) {
        if (token.value() == "eval") {
            if (tokens->front().type() != Token::Type::SPACE)
                throw Exception("Invalid token, expecting ' ' after '$(eval "
                                "' but got: " + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::DOLLAR)
                throw Exception("Invalid token, expecting '$(cal ' after "
                                "'$(eval ' but got: "
                                + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::OPEN_PARENTHESIS)
                throw Exception("Invalid token, expecting '$(cal ' after "
                                "'$(eval ' but got: "
                                + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::ALPHANUM ||
                tokens->front().value() != "call") {
                throw Exception("Invalid token, expecting '$(cal ' after "
                                "'$(eval ' but got: "
                                + tokens->front().value());
            }

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::SPACE)
                throw Exception("Invalid token, expecting ' ' after '$(eval "
                                "$(call ' but got: "
                                + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::ALPHANUM)
                throw Exception("Invalid token, expecting Function after "
                                "'$(eval $(call ' but got: "
                                + tokens->front().value());

            token = tokens->front();
            tokens->pop_front();
            const auto it = _functions.find(token.value());
            if (it == _functions.end())
                throw Exception("Invalid token, expecting a Function but "
                                "got: " + token.value());

            const auto func = it->second;
            ret = func(*_attributes, _file, tokens);
        }
        else {
            //ERROR
        }
    }

    token = tokens->front();
    tokens->pop_front();
    if (token.type() == Token::Type::SPACE) {
        token = tokens->front();
        tokens->pop_front();
    }

    if (token.type() != Token::Type::CLOSE_PARENTHESIS)
        throw Exception(R"(Invalid token, expecting ')' but got: )"
                        + token.value());

    token = tokens->front();
    tokens->pop_front();
    if (token.type() == Token::Type::SPACE) {
        token = tokens->front();
        tokens->pop_front();
    }

    if (token.type() != Token::Type::CLOSE_PARENTHESIS)
        throw Exception(R"(Invalid token, expecting ')' but got: )"
                        + token.value() + " - " + _file);

    return ret;
}

int main(int argc, char **argv)
{
    std::string file("/Users/leobispo/workspace/rtbkit/rtbkit/rtbkit.mk");
    try {
        MKParser parser(file, { "googleurl", "tinyxml2", "leveldb", "jml",
                                "soa"});

        parser.run();
    } catch (_Exception &e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
