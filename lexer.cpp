/*
# COMMENT

VAR := END
VAR := LINE
VAR := FILE LINE
LINE \
*/

#include <fstream>
#include <sstream>
#include <iostream>

#include <string>
#include <deque>
#include <vector>
#include <memory>
#include <unordered_map>

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

#define Exception(value) _Exception("LINE=" + std::to_string(__LINE__) + " " + value)

class Token
{
    public:
        enum class Type {
            DOLLAR,
            OPEN_PARENTHESIS,
            CLOSE_PARENTHESIS,
            IFEQ,
            ENDIF,
            COMMA,
            ASSIGN,
            ALPHANUM,
            BACKSLASH,
            SPACE,
            NUMBER,
            NEW_LINE,
            INVALID,
            END
        };

        Token();
        Token(Type type, const std::string &value);

        const std::string &value() const;
        Type type() const;
    private:
        Type _type;
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

class AttributeAST;
class LibraryAST;
class NodeJsAddonAST;
class NodeJsTestAST;
class TestAST;
class SubMakeAST;
class VOWSCoffeeTestAST;

class FileParser
{
    public:
        FileParser(const std::string &file);

        std::string parse(bool writeInline = false);

        static std::unordered_map<std::string, std::string> _libraryMap;
    private:
        mutable char _lastChar = 0;

        std::string  _file;

        std::unordered_map<std::string, std::shared_ptr<AttributeAST>> _attributes;
        std::vector<std::shared_ptr<ExtAST>> _AST;

        std::shared_ptr<AttributeAST> parseAttribute(std::deque<Token> *tokens) const;
        std::vector<std::string> expandAttribute(std::deque<Token> *tokens) const;
        std::vector<std::string> parseValues(std::deque<Token> *tokens) const;

        
        std::vector<std::vector<std::string>> parseFunctionArgs(std::deque<Token> *tokens, 
                                                                size_t number) const;


        std::shared_ptr<ExtAST> parseFunction(std::deque<Token> *tokens) const;
        std::shared_ptr<LibraryAST> parseLibrary(std::deque<Token> *tokens) const;
        std::shared_ptr<NodeJsAddonAST> parseNodeJsAddon(std::deque<Token> *tokens) const;
        std::shared_ptr<NodeJsTestAST> parseNodeJsTest(std::deque<Token> *tokens) const;
        std::shared_ptr<TestAST> parseTest(std::deque<Token> *tokens) const;
        std::shared_ptr<SubMakeAST> parseSubMake(std::deque<Token> *tokens) const;
        std::shared_ptr<VOWSCoffeeTestAST> parseVOWSCoffeeTest(std::deque<Token> *tokens) const;

        std::deque<Token> lexer() const;
        Token nextToken(std::ifstream &is) const;
};

class VOWSCoffeeTestAST : public ExtAST
{
    public:
        VOWSCoffeeTestAST(const std::string &name, const std::vector<std::string> &dependencies,
                          const std::vector<std::string> &options, const std::string &target,
                          const std::vector<std::string> &testOptions);
 

    private:
        std::string _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _options;
        std::string _target;
        std::vector<std::string> _testOptions;
        std::string codeGen() const;
};

VOWSCoffeeTestAST::VOWSCoffeeTestAST(const std::string &name, const std::vector<std::string> &dependencies,
                                     const std::vector<std::string> &options, const std::string &target,
                                     const std::vector<std::string> &testOptions)
    : _name(name), _dependencies(dependencies), _options(options), _target(target), _testOptions(testOptions)
{
}

std::string VOWSCoffeeTestAST::codeGen() const
{
    return "";
}

class NodeJsAddonAST : public ExtAST
{
    public:
        NodeJsAddonAST(const std::string &name, const std::vector<std::string> &sources,
                   const std::vector<std::string> &dependencies,
                   const std::vector<std::string> &otherJs);

    private:
        std::string _name;
        std::vector<std::string> _sources;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _otherJs;

        std::string codeGen() const;
};

NodeJsAddonAST::NodeJsAddonAST(const std::string &name, const std::vector<std::string> &sources,
                       const std::vector<std::string> &dependencies,
                       const std::vector<std::string> &otherJs)
    : _name(name), _sources(sources), _dependencies(dependencies), _otherJs(otherJs)
{
}

std::string NodeJsAddonAST::codeGen() const
{
    std::string code;

    auto libName = _name;

    code = "noinst_LTLIBRARIES += " + libName + ".la\n\n";
    code += libName + "_la_LDFLAGS = $(NODEJS_LIBTOOL_FLAGS)\n";
    code += libName + "_la_CXXFLAGS = \n"; //TODO: FIXME!!

    code += libName + "_la_SOURCES = \\\n";
    size_t i;
    for (i = 0; i < _sources.size() - 1; ++i)
        code += "  " + _sources.at(i) + " \\\n";

    if (!_sources.empty())
        code += "  " + _sources.at(i) + "\n";

    code += "\n" + libName + "_la_LIBADD = \\\n";

    for (i = 0; i < _dependencies.size() - 1; ++i) {
        auto t = FileParser::_libraryMap.find(_dependencies.at(i));
        if (t != FileParser::_libraryMap.end())
            code += "  " + t->second + " \\\n";
        else
            code += "  " + _dependencies.at(i) + " \\\n";
    }

    if (!_dependencies.empty()) {
        auto t = FileParser::_libraryMap.find(_dependencies.at(i));
        if (t != FileParser::_libraryMap.end())
            code += "  " + t->second + " \n";
        else
            code += "  " + _dependencies.at(i) + " \n";
    }
    code += "\n";

    return code;
}
 
class TestAST : public ExtAST
{
    public:
        TestAST(const std::string &name, const std::vector<std::string> &dependencies,
                const std::vector<std::string> &style, const std::vector<std::string> &targets);

    private:
        std::string _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _style;
        std::vector<std::string> _targets;

        std::string codeGen() const;
};

TestAST::TestAST(const std::string &name, const std::vector<std::string> &dependencies,
                 const std::vector<std::string> &style, const std::vector<std::string> &targets)
    : _name(name), _dependencies(dependencies), _style(style), _targets(targets)
{
}

std::string TestAST::codeGen() const
{
    std::string code;
    code += "CC_TESTS += " + _name + "\n";
    code += _name + "_SOURCES = " + _name + ".cc\n";
    code += _name + "_CXXFLAGS =\n";

    code += _name + "_LADD = \\\n";

    size_t i = 0;
    for (i = 0; i < _dependencies.size() - 1; ++i) {
        auto t = FileParser::_libraryMap.find(_dependencies.at(i));
        if (t != FileParser::_libraryMap.end())
            code += "  " + t->second + " \\\n";
        else
            code += "  " + _dependencies.at(i) + " \\\n";
    }

    if (!_dependencies.empty()) {
        auto t = FileParser::_libraryMap.find(_dependencies.at(i));
        if (t != FileParser::_libraryMap.end())
            code += "  " + t->second + " \n";
        else
            code += "  " + _dependencies.at(i) + " \n";
    }
    code += "\n";

    return code;
}

class NodeJsTestAST : public ExtAST
{
    public:
        NodeJsTestAST(const std::string &name, const std::vector<std::string> &dependencies,
                      const std::vector<std::string> &options, const std::string &testName,
                      const std::vector<std::string> &testOptions);
    private:
        std::string _name;
        std::vector<std::string> _dependencies;
        std::vector<std::string> _options;
        std::string _testName;
        std::vector<std::string> _testOptions;
        
        std::string codeGen() const;
};

NodeJsTestAST::NodeJsTestAST(const std::string &name, const std::vector<std::string> &dependencies,
                             const std::vector<std::string> &options, const std::string &testName,
                             const std::vector<std::string> &testOptions)
    : _name(name), _dependencies(dependencies), _options(options), _testName(testName),
      _testOptions(testOptions)
{
}
 
std::string NodeJsTestAST::codeGen() const
{
    return "";
}

class LibraryAST : public ExtAST
{
    public:
        LibraryAST(const std::string &name, const std::vector<std::string> &sources,
                   const std::vector<std::string> &dependencies, const std::string & output,
                   const std::string &extension, const std::string &buildName);

    private:
        std::string _name;
        std::vector<std::string> _sources;
        std::vector<std::string> _dependencies;
        std::string _output;
        std::string _extension;
        std::string _buildName;

        std::string codeGen() const;
};

LibraryAST::LibraryAST(const std::string &name, const std::vector<std::string> &sources,
                       const std::vector<std::string> &dependencies, const std::string & output,
                       const std::string &extension, const std::string &buildName)
    : _name(name), _sources(sources), _dependencies(dependencies), _extension(extension),
      _output(output), _buildName(buildName)
{
}

std::string LibraryAST::codeGen() const
{
    std::string code;

    auto libName = "lib" + (_output.empty() ? _name : _output);

    code = "lib_LTLIBRARIES += " + libName + ".la\n\n";

    code += libName + "_la_LDFLAGS = -avoid-version\n";
    code += libName + "_la_CXXFLAGS = \n"; //$(ZMQ_FLAGS) $(MONGODB_FLAGS)

    code += libName + "_la_SOURCES = \\\n";
    size_t i;
    for (i = 0; i < _sources.size() - 1; ++i)
        code += "  " + _sources.at(i) + " \\\n";

    if (!_sources.empty())
        code += "  " + _sources.at(i) + "\n";

    code += "\n" + libName + "_la_LIBADD = \\\n";

    for (i = 0; i < _dependencies.size() - 1; ++i) {
        auto t = FileParser::_libraryMap.find(_dependencies.at(i));
        if (t != FileParser::_libraryMap.end())
            code += "  " + t->second + " \\\n";
        else
            code += "  " + _dependencies.at(i) + " \\\n";
    }

    if (!_dependencies.empty()) {
        auto t = FileParser::_libraryMap.find(_dependencies.at(i));
        if (t != FileParser::_libraryMap.end())
            code += "  " + t->second + " \n";
        else
            code += "  " + _dependencies.at(i) + " \n";
    }
    return code;
}
 
class SubMakeAST : public ExtAST
{
    public:
        SubMakeAST(const std::string &name, const std::string &basedir,
                   const std::string &dir, const std::string &makefile);

    private:
        std::string _name;
        std::string _basedir;
        std::string _dir;
        std::string _makefile;

        std::string codeGen() const;
};

SubMakeAST::SubMakeAST(const std::string &name, const std::string &basedir,
                       const std::string &dir, const std::string &makefile)
    : _name(name), _basedir(basedir), _dir(dir), _makefile(makefile)
{
}

std::string SubMakeAST::codeGen() const
{
    auto dir = (_dir.empty() ? _name : _dir);
    auto file = _basedir + dir + "/" + (_makefile.empty() ? (dir + ".mk") : _makefile);

    FileParser parser(file);

    auto code = parser.parse(true);

    if (dir == "testing")
        return code;
    else {
        //TODO: Write to the file!!
        return "";
    }
}
 
class AttributeAST : public ExtAST
{
    public:
        AttributeAST(const std::string &key, const std::vector<std::string> &value);

        const std::string &key() const;
        const std::vector<std::string> &value() const;
    private:
        std::string _key;
        std::vector<std::string> _value;

        std::string codeGen() const;
};

AttributeAST::AttributeAST(const std::string &key, const std::vector<std::string> &value)
    : _key(key), _value(value)
{
}

const std::string &AttributeAST::key() const
{
    return _key;
}

const std::vector<std::string> &AttributeAST::value() const
{
    return _value;
}

std::string AttributeAST::codeGen() const
{
    return "Attribute\n";
}
 
std::unordered_map<std::string, std::string> FileParser::_libraryMap = {
    { "ACE", "$(ACE_LIB)" },
    { "boost_thread", "$(BOOST_THREAD_LIB)" },
    { "boost_regex", "$(BOOST_REGEX_LIB)" },
    { "zeromq", "$(ZMQ_LIB)" },
    { "lzma", "$(LZMA_LIB)" },
    { "boost_filesystem", "$(BOOST_FILESYSTEM_LIB)" },
    { "mongoclient", "$(MONGODB_LIB)" },
    { "boost_program_options", "$(BOOST_PROGRAM_OPTIONS_LIB)" }
};

FileParser::FileParser(const std::string &file) : _file(file)
{
}

Token FileParser::nextToken(std::ifstream &is) const
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

    if (isalpha(c) || c == '_') {
        std::string alphanum;
        do {
            alphanum.push_back(c);
            c = is.get();
        } while (isalnum(c) || c == '_' || c == '.');

        _lastChar = c;
        return Token(Token::Type::ALPHANUM, alphanum);
    }

    if (c == ':') {
        c = is.get();
        if (c == '=')
            return Token(Token::Type::ASSIGN, ":=");
    }

    throw Exception("Invalid Token");
}

std::deque<Token> FileParser::lexer() const
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

std::shared_ptr<ExtAST> FileParser::parseFunction(std::deque<Token> *tokens) const
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
                throw Exception(R"(Invalid token, expecting ' ' after '$(eval ' but got: )"
                                + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::DOLLAR)
                throw Exception(R"(Invalid token, expecting '$(cal ' after '$(eval ' but got: )"
                                + tokens->front().value());
               
            tokens->pop_front();
            if (tokens->front().type() != Token::Type::OPEN_PARENTHESIS)
                throw Exception(R"(Invalid token, expecting '$(cal ' after '$(eval ' but got: )"
                                + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::ALPHANUM || tokens->front().value() != "call")
                throw Exception(R"(Invalid token, expecting '$(cal ' after '$(eval ' but got: )"
                                + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::SPACE)
                throw Exception(R"(Invalid token, expecting ' ' after '$(eval $(call ' but got: )"
                                + tokens->front().value());

            tokens->pop_front();
            if (tokens->front().type() != Token::Type::ALPHANUM)
                throw Exception(R"(Invalid token, expecting Function after '$(eval $(call ' but got: )"
                                + tokens->front().value());

            token = tokens->front();
            tokens->pop_front();
            if (token.value() == "library")
                ret = parseLibrary(tokens);
            else if (token.value() == "include_sub_make")
                ret = parseSubMake(tokens);
            else if (token.value() == "nodejs_addon")
                ret = parseNodeJsAddon(tokens);
            else if (token.value() == "nodejs_test")
                ret = parseNodeJsTest(tokens);
            else if (token.value() == "test")
                ret = parseTest(tokens);
            else if (token.value() == "vowscoffee_test")
                ret = parseVOWSCoffeeTest(tokens);
            else
                throw Exception(R"(Invalid token, expecting library, include_sub_make or nodejs_addon but got: )"
                                + tokens->front().value());
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
                        + token.value());

    return ret;
}

std::vector<std::vector<std::string>> FileParser::parseFunctionArgs(std::deque<Token> *tokens,
                                                                    size_t number) const

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
  
        value = parseValues(tokens);
    }

    return values;
}

/*
 * # $(1): name of the addon
 * # $(2): source files to include in the addon
 * # $(3): libraries to link with
 * # $(4): other node.js addons that need to be linked in with this one
 */
std::shared_ptr<NodeJsAddonAST> FileParser::parseNodeJsAddon(std::deque<Token> *tokens) const
{
    auto args = parseFunctionArgs(tokens, 4);

    if (args.at(0).size() != 1)
        throw Exception("Must pass only 1 name");

    if (args.at(1).empty())
        throw Exception("Must pass at least 1 source file");

    return std::make_shared<NodeJsAddonAST>(args.at(0).at(0), args.at(1), args.at(2), args.at(3));
}

/*
 * # add a test case
 * # $(1) name of the test
 * # $(2) libraries to link with
 * # $(3) test style.  boost = boost test framework, and options: manual, valgrind
 * # $(4) testing targets to add it to
 */
std::shared_ptr<TestAST> FileParser::parseTest(std::deque<Token> *tokens) const
{
    auto args = parseFunctionArgs(tokens, 4);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    return std::make_shared<TestAST>(args.at(0).at(0), args.at(1), args.at(2), args.at(3));
}
 
/*
 * # $(1) name of the test (the javascript file that contains the test case)
 * # $(2) node.js modules on which it depends
 * # $(3) options to the node executable
 * # $(4) test name
 * # $(5) test options
 */
std::shared_ptr<NodeJsTestAST> FileParser::parseNodeJsTest(std::deque<Token> *tokens) const
{
    auto args = parseFunctionArgs(tokens, 5);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(3).size() > 1)
        throw Exception("Must have maximum 1 testName");

    auto testName = args.at(3).empty() ? "" : args.at(3).at(0);

    return std::make_shared<NodeJsTestAST>(args.at(0).at(0), args.at(1), args.at(2), testName,
                                           args.at(4));
}

/*
 * # arg 1: name
 * # arg 2: dir (optional, is the same as $(1) if not given)
 * # arg 3: makefile (optional, is $(2)/$(1).mk if not given)
 */
std::shared_ptr<SubMakeAST> FileParser::parseSubMake(std::deque<Token> *tokens) const
{
    auto args = parseFunctionArgs(tokens, 5);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(1).size() > 1)
        throw Exception("Must have maximum 1 dir");

    if (args.at(2).size() > 1)
        throw Exception("Must have maximum 1 makefile");

    auto dir = args.at(1).empty() ? "" : args.at(1).at(0);
    auto makefile = args.at(2).empty() ? "" : args.at(2).at(0);

    return std::make_shared<SubMakeAST>(args.at(0).at(0), _file.substr(0, _file.find_last_of("/")) + "/",
                                        dir, makefile);
}

/*
 * # $(1): name of the library
 * # $(2): source files to include in the library
 * # $(3): libraries to link with
 * # $(4): output name; default lib$(1)
 * # $(5): output extension; default .so
 * # $(6): build name; default SO
 */
std::shared_ptr<LibraryAST> FileParser::parseLibrary(std::deque<Token> *tokens) const
{
    auto args = parseFunctionArgs(tokens, 6);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(1).empty())
        throw Exception("Must pass at least 1 source file");

    if (args.at(3).size() > 1)
        throw Exception("Must have maximum 1 output name");

    if (args.at(4).size() > 1)
        throw Exception("Must have maximum 1 output extension");

    if (args.at(5).size() > 1)
        throw Exception("Must have maximum 1 build namer");

    auto output = args.at(3).empty() ? "" : args.at(3).at(0);
    auto extension = args.at(4).empty() ? "" : args.at(4).at(0);
    auto buildName = args.at(5).empty() ? "" : args.at(5).at(0);

    //TODO: CHANGE ME TO CHECK FOR BUILD NAME - And check if the path is absolute or relative!!
    auto libPath = _file.substr(0, _file.find_last_of("/")) + "/" + "lib" + args.at(0).at(0) + ".la";
    _libraryMap[args.at(0).at(0)] = libPath;

    return std::make_shared<LibraryAST>(args.at(0).at(0), args.at(1), args.at(2),
                                        output, extension, buildName);
}

/**
 * # $(1) name of the test (the javascript file that contains the test case)
 * # $(2) node.js modules on which it depends
 * # $(3) options to the vows executable
 * # $(4) test target
 * # $(5) test options (eg, manual)
 */
std::shared_ptr<VOWSCoffeeTestAST> FileParser::parseVOWSCoffeeTest(std::deque<Token> *tokens) const
{
    auto args = parseFunctionArgs(tokens, 5);

    if (args.at(0).size() != 1)
        throw Exception("Must have only 1 name");

    if (args.at(3).size() > 1)
        throw Exception("Must have maximum 1 testName");

    auto testName = args.at(3).empty() ? "" : args.at(3).at(0);

    return std::make_shared<VOWSCoffeeTestAST>(args.at(0).at(0), args.at(1), args.at(2), testName,
                                               args.at(4));
}

std::vector<std::string> FileParser::parseValues(std::deque<Token> *tokens) const
{
    if (tokens->front().type() == Token::Type::END)
        throw Exception(R"(Not expecting End of File)");

    std::vector<std::string> values;
    do {
        switch (tokens->front().type()) {
            case Token::Type::SPACE:
                break;
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
                auto tmpValues = expandAttribute(tokens);
                values.insert(values.end(), tmpValues.begin(), tmpValues.end());
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

std::shared_ptr<AttributeAST> FileParser::parseAttribute(std::deque<Token> *tokens) const
{
    auto attr = tokens->front().value();
    tokens->pop_front();

    if (tokens->front().type() == Token::Type::END)
        throw Exception(R"(Not expecting End of File)");

    if (tokens->front().type() == Token::Type::SPACE)
        tokens->pop_front();

    if (tokens->front().type() != Token::Type::ASSIGN)
        throw Exception(R"(Invalid token, expecting := but got: )"
                        + tokens->front().value());

    tokens->pop_front();

    auto values = parseValues(tokens);

    if (values.empty())
        throw Exception(R"(Attribute cannot be empty)");

    return std::make_shared<AttributeAST>(attr, values);
}

std::vector<std::string> FileParser::expandAttribute(std::deque<Token> *tokens) const
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
        throw Exception("Invalid token, expecting VARIABLE after '$(' but got: "
                        + tokens->front().value());

    auto variable = tokens->front().value();

    tokens->pop_front();
    if (tokens->front().type() != Token::Type::CLOSE_PARENTHESIS)
        throw Exception("Invalid token, expecting ) but got: "
                        + tokens->front().value());

    tokens->pop_front();
    auto v = _attributes.find(variable);
    if (v != _attributes.end())
        return v->second->value();

    throw Exception("Variable " + variable + " not defined");
}

std::string FileParser::parse(bool writeInline)
{
    std::deque<Token> tokens = lexer();

    while (!tokens.empty()) {
        switch (tokens.front().type()) {
            case Token::Type::NEW_LINE:
            case Token::Type::SPACE:
                tokens.pop_front();
            break;
            case Token::Type::ALPHANUM:
                if (tokens.front().value() == "ifeq") {
                    // MUST HANDLE THE ENDIF
                }
                else if (tokens.front().value() != "endif") {
                    auto attr = parseAttribute(&tokens);
                    _attributes[attr->key()] = attr;
                }
            break;
            case Token::Type::DOLLAR:
            {
                tokens.pop_front();
                _AST.push_back(parseFunction(&tokens));
            }
            break;
            case Token::Type::END:
                tokens.pop_front();
            break;
            default:
                throw Exception("Not expecting token : " + tokens.front().value());
        }
    }

    std::string code;
    if (!writeInline) {
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
    }
    else {
        code += "\nCC_TESTS =\n\n";
    }

    for (const auto &element : _AST)
        code += element->codeGen();

    if (!writeInline) {
        code += "node_prefix=$(exec_prefix)/node_modules\n\n";
        code += "install-exec-hook:\n";
        code += "\tmkdir -p $(node_prefix)\n";
        code += "\tcp -rf .libs/*.node $(node_prefix)\n";
        code += "uninstall-hook:\n";
        code += "\tfor i in $(noinst_LTLIBRARIES); do lib=`echo \"$$i\" | sed 's/\\.la/\\.node/g'`; rm $(node_prefix)/$$lib; done\n";
        code += "\tif find \"$(node_prefix)\" -maxdepth 0 -empty | read; then rm -rf $(node_prefix); fi\n";
    }
    else {
        code += "TESTS_ENVIRONMENT = $(abs_top_builddir)/test_driver.sh NODE=$(NODEJS) VOWS=$(VOWS) "
                "NODE_LIBS=\"$(noinst_LTLIBRARIES)\"\n";
        code += "TESTS = $(CC_TESTS) $(abs_top_builddir)/runjstest.sh\n\n";
    }

    return code;
}

int main(int argc, char **argv)
{
    std::string file("/Users/leobispo/workspace/rtbkit/soa/logger/logger.mk");

    try {
        FileParser parser(file);
        std::cout << parser.parse()  << std::endl;
    } catch (_Exception &e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
