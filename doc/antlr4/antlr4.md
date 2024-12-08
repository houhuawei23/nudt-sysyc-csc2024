### antlr4 Class Relationshape

```c++
/* antlr4 Class Relationshape */
// Token Classes

  Token <- WritableToken <- CommonToken

// Stream Classes

             /- CharStream <- ANTLRInputStream <- ANTLRFileStream
            /             \-- UnbufferedCharStream
  IntStream 
            \- BufferedTokenStream <- CommonTokenStream
             \- UnbufferedTokenStream

// Lexer and Parser Classes

  TokenSource <- Lexer <- ExpressionLexer
                /
            /--/                
  Recognizer 
            \--- Parser <- ExpressionParser

// Parse Trees and Rule Contexts

                             /- ErrorNode
            /- TerminalNode /-- TerminalNodempl <- ErrorNodelmpl
  ParseTree
            \- RuleContext <- ParserRuleContext <- CompUnitContext
```

### antlr4 api

```c++
antlr4::Token {
    getText() -> size_t
    getType()
    getLine()
    getCharPositionInLine()
    getChannel()
    getTokenIndex()
    getStartIndex()
    getStopIndex()
    getTokenSource() -> TokenSource*
    getInputStream() -> CharStream*
    toString() -> string
}

antlr4::IntStream { // Pure Virtual
    virtual void consume() = 0;
    virtual size_t LA(ssize_t i) = 0;
    virtual ssize_t mark() = 0;
    virtual void release(ssize_t marker) = 0;
    virtual size_t index() = 0;
    virtual void seek(size_t index) = 0;
    virtual size_t size() = 0;
    virtual std::string getSourceName() const = 0;
}

antlr4::RuleContext : public tree::ParseTree {
    bool is(const tree::ParseTree &parseTree);
    virtual int depth();
    virtual bool isEmpty();
    virtual misc::Interval getSourceInterval() override;

    virtual std::string getText() override;

    virtual size_t getRuleIndex() const;
    // return the outer alternative number used to match the input
    virtual size_t getAltNumber() const; 
    virtual std::string toStringTree(Parser *recog, bool pretty = false) override;
    virtual std::string toStringTree(bool pretty = false) override;
    virtual std::string toString() override;
}

antlr4::ParserRuleContext : public RuleContext {
    Token *start;
    Token *stop;
    virtual void copyFrom(ParserRuleContext *ctx);
    tree::TerminalNode* getToken(size_t ttype, std::size_t i) const;

    std::vector<tree::TerminalNode*> getTokens(size_t ttype) const;

    template<typename T>
    T* getRuleContext(size_t i) const {...};

    template<typename T>
    std::vector<T*> getRuleContexts() const {...};

    virtual misc::Interval getSourceInterval() override;

    Token* getStart() const;
    Token* getStop() const;

    virtual std::string toInfoString(Parser *recognizer);
}



antlr4::tree::ParseTree {
    ParseTree *parent;
    vector<ParseTree *> children;
    string toStringTree(bool pretty = false)
    string toString();
    string getText();
    misc::Interval getSourceInterval();
}


```

### antlr4 api

```C++
Token
    getType()             // The token's type
    getLine()             // The number of the line containing the token
    getText()             // The text associated with the token
    getCharPositionInLine() // The line position of the token's first character
    getTokenIndex()         // The index of the token in the input stream
    getChannel()         // The channel that provides the token to the parser
    getStartIndex()         // The token's starting character index
    getStopIndex()         // The token's last character index
    getTokenSource()     // A pointer to the TokenSource that created the token
    getInputStream()     // A pointer to the CharStream that provided the token's characters

IntStream
    consume()                   // Accesses and consumes the current element
    LA(ssize_t i)               // Reads the element i positions away
    mark()                      // Returns a handle that identifies a position in the stream
    release(ssize_t marker)     // Releases the position handle
    index()                     // Returns the index of the upcoming element
    seek(ssize_t index)         // Set the input cursor to the given position
    size()                      // Returns the number of elements in the stream


Parser
    /* Token Functions */
    consume()                       // Consumes and returns the current token
    getCurrentToken()               // Returns the current token
    isExpectedToken(size_t symbol)  // Checks whether the current token has the given type
    getExpectedTokens()             // Provides tokens in the current context
    isMatchedEOF()                  // Identifies the current token is EOF
    createTerminalNode(Token* t)    // Adds a new terminal node to the tree
    createErrorNode(Token* t)       // Adds a new error node to the tree
    match(size_t ttype)             // Returns the token if it matches the given type
    matchWildcard()                 // Match the current token as a wildcard
    getInputStream()                // Returns the parser's IntStream
    setInputStream(IntStream* is)   // Sets the parser's IntStream
    getTokenStream()                // Returns the parser's TokenStream
    setTokenStream(TokenStream* ts) // Sets the parser's TokenStream
    getTokenFactory()               // Returns the parser's TokenFactory
    reset()                         // Resets the parser's state

    /* Parse Tree and Listener Functions */
    getBuildParseTree()                         // Checks if a parse tree will be constructed during parsing
    setBuildParseTree(bool b)                   // Identifies if a parse tree should be constructed
    getTrimParseTree()                          // Checks if the parse tree is trimmed during parsing
    setTrimParseTree(bool t)                    // Identifies the parse tree should be trimmed during parsing
    getParseListeners()                         // Returns the vector containing the parser's listeners
    addParseListener(ParseTreeListener* ptl)    // Adds a listener to the parser
    removeParseListener(ParseTreeListener* ptl) // Removes a listener from the parser
    removeParseListeners()                      // Removes all listeners from the parser

    /* Error Functions */
    getNumberOfSyntaxErrors()                   // Returns the number of syntax errors
    getErrorHandler()                           // Returns the parser's error handler
    setErrorHandler(handler)                    // Sets the parser's error handler
    notifyErrorListeners(string msg)            // Sends a message to the parser's error listeners
    notifyErrorListeners(Token* t, string msg, exception_ptr e)  // Sends data to the parser's error listeners

    /* Rule Functions */
    enterRule(ParserRuleContext* ctx, size_t state, size_t index)  // Called upon rule entry
    exitRule()                                  // Called upon rule exit
    triggerEnterRuleEvent()                     // Notify listeners of rule entry
    triggerExitRuleEvent()                      // Notify listeners of rule exit
    getRuleIndex(string rulename)               // Identify the index of the given rule
    getPrecedence()                             // Get the precedence level of the topmost rule
    getContext()                                // Returns the context of the current rule
    setContext(ParserRuleContext* ctx)          // Sets the parser's current rule
    getInvokingContext(size_t index)            // Returns the context that invoked the current context
    getRuleInvocationStack()                    // Returns a list of rules processed up to the current rule
    getRuleInvocationStack(RuleContext* ctx)    // Returns a list of rules processed up to the given rule
```


## ATN AST Transition Network

In ANTLR4 (Another Tool for Language Recognition), `ATN` stands for **Abstract Syntax Tree (AST) Transition Network**. The ATN is a core component of ANTLR's internal mechanism for parsing and recognizing languages.

### **What is ATN?**
- **Abstract Syntax Tree (AST):** While the ATN itself is not an AST, it is used in the process of building one. The ATN is a model that represents the grammar rules of the language you are parsing. It is essentially a finite state machine that drives the parsing process.
- 抽象语法树 (AST)：虽然 ATN 本身不是 AST，但在构建 AST 的过程中使用了它。 ATN 是一个模型，代表您正在解析的语言的语法规则。它本质上是一个驱动解析过程的有限状态机。

- **Transition Network:** The ATN is composed of a series of states and transitions between these states. Each state represents a position in the parsing process, and transitions are driven by the input symbols (tokens).
- 转换网络： ATN 由一系列状态和这些状态之间的转换组成。每个状态代表解析过程中的一个位置，并且转换由输入符号（令牌）驱动。

### **Role of ATN in ANTLR4**
1. **Parsing Strategy:** The ATN is used by ANTLR4 to implement its parsing strategy, which is a combination of LL(*) and SLL (Simple LL) parsing. It helps in handling recursive descent parsing with infinite lookahead, making it powerful and flexible in recognizing complex language constructs.
   - 解析策略： ANTLR4使用ATN来实现其解析策略，该策略是LL(*)和SLL(Simple LL)解析的组合。它有助于处理具有无限前瞻的递归下降解析，使其在识别复杂语言结构方面强大而灵活。

2. **Prediction:** ANTLR uses the ATN to make decisions about which rule to apply when there are multiple possibilities. This process is known as **prediction**, where ANTLR looks ahead in the input stream to decide which path to take in the ATN.
   - 错误处理： ATN 对于错误处理也至关重要。当解析器遇到意外标记时，ATN 会帮助确定最佳恢复方式，通常是找到仍可成功解析的有效路径。

3. **Error Handling:** The ATN is also crucial for error handling. When the parser encounters an unexpected token, the ATN helps determine the best way to recover, often by finding a valid path that can still lead to successful parsing.
   - 错误处理： ATN 对于错误处理也至关重要。当解析器遇到意外标记时，ATN 会帮助确定最佳恢复方式，通常是找到仍可成功解析的有效路径。

4. **Efficiency:** The ATN allows ANTLR4 to be more efficient than its predecessors. By using the ATN and a prediction mechanism, ANTLR can handle large and complex grammars more effectively, reducing the need for backtracking and improving performance.
   - 效率： ATN 使 ANTLR4 比其前代产品更加高效。通过使用ATN和预测机制，ANTLR可以更有效地处理大型且复杂的语法，减少回溯的需要并提高性能。
### **How ATN Works**
- **States:** The ATN consists of various states that correspond to different points in the grammar rules. For example, entering a new rule, exiting a rule, or matching a particular token.
- 状态： ATN 由对应于语法规则中不同点的各种状态组成。例如，输入新规则、退出规则或匹配特定标记。

- **Transitions:** Transitions between these states occur based on the input tokens. The transitions guide the parser through the grammar, matching input to the expected language structure.
- 转换：这些状态之间的转换根据输入标记发生。转换引导解析器完成语法，将输入与预期的语言结构进行匹配。

- **Prediction Mode:** ANTLR can operate in different prediction modes (`SLL`, `LL`), using the ATN to determine the correct parsing strategy dynamically.
- 预测模式： ANTLR可以在不同的预测模式（ SLL 、 LL ）下运行，利用ATN动态确定正确的解析策略。

### **Practical Use in ANTLR4**
As a user of ANTLR4, you generally don't interact directly with the ATN. It operates behind the scenes as part of the parser's internal mechanics. However, understanding the ATN can help you grasp how ANTLR4 manages to efficiently parse and recognize the structure of the language defined by your grammar.

For advanced users, such as those tweaking performance or debugging complex grammars, knowledge of the ATN and how it influences parsing can be valuable. It helps in understanding the predictions made by ANTLR and how to optimize or debug grammar rules.

### **Conclusion**
In summary, the ATN in ANTLR4 is a crucial internal structure that represents the finite state machine driving the parsing process. It is central to ANTLR4's ability to efficiently and accurately parse languages, making it a powerful tool for language recognition.