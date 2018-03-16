/*
 Copyright Disney Enterprises, Inc.  All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License
 and the following modification to it: Section 6 Trademarks.
 deleted and replaced with:

 6. Trademarks. This License does not grant permission to use the
 trade names, trademarks, service marks, or product names of the
 Licensor and its affiliates, except as required for reproducing
 the content of the NOTICE file.

 You may obtain a copy of the License at
 http://www.apache.org/licenses/LICENSE-2.0
*/
#ifndef _ExprFuncX_h_
#define _ExprFuncX_h_

#include "ExprType.h"
#include "Vec.h"
#include "ExprNode.h"

namespace SeExpr2 {
class ExprFuncDeclaration;
class ExprFuncNode;
class ExprFuncSimple;
class ExprVarEnv;
class Interpreter;
typedef std::map<std::string, double> Statistics;

//! Extension function spec, used for complicated argument custom functions.
/** Provides the ability to handle all argument type checking and processing manually.
    Derive from this class and then make your own ExprFunc that takes this object.
    This is necessary if you need string arguments or you have variable numbers of
    arguments.  See ExprBuiltins.h for some examples */
class ExprFuncX {
  public:
    //! Create an ExprFuncX. If the functions and type checking you implement
    //! is thread safe your derived class should call this with true. If not,
    //! then false.  If you mark a function as thread unsafe,  and it is used
    //! in an expression then bool Expression::isThreadSafe() will return false
    //! and the controlling software should not attempt to run multiple threads
    //! of an expression.
    ExprFuncX(const bool threadSafe) : _threadSafe(threadSafe) {}

    /** prep the expression by doing all type checking argument checking, etc. */
    virtual ExprType prep(ExprFuncNode* node, bool scalarWanted, ExprVarEnvBuilder& env) const = 0;
    virtual ExprType type() const { return _type; }

    /** evaluate the expression. the given node is where in the parse tree
        the evaluation is for */
    // virtual void eval(const ExprFuncNode* node, Vec3d& result) const = 0;
    //! Build an interpreter to evaluate the expression
    virtual int buildInterpreter(const ExprFuncNode* node, Interpreter* interpreter) const = 0;
    virtual ~ExprFuncX() {}

    bool isThreadSafe() const { return _threadSafe; }

    /// Return memory usage of a funcX in bytes.
    virtual size_t sizeInBytes() const { return 0; }

    /// Give this function a chance to populate its statistics
    virtual void statistics(Statistics& /*statistics*/) const {}

  protected:
    bool _isScalar;
    ExprType _type;

  private:
    bool _threadSafe;
};

// This is used for most plugin functions
class ExprFuncSimple : public ExprFuncX {
  public:
    ExprFuncSimple(const bool threadSafe) : ExprFuncX(threadSafe) {}

    class ArgHandle {
      public:
        ArgHandle(const int* opData, double* fp, char** c, std::vector<int>& /*callStack*/, const char* v)
            : outFp(fp[opData[2]]), outStr(c[opData[2]]), data(reinterpret_cast<ExprFuncNode::Data*>(c[opData[1]])),
              varBlock(v), _nargs((int)fp[opData[3]])  // TODO: put the value in opData rather than fp
              ,
              opData(opData + 4)  // TODO: would be good not to have to convert to int!
              ,
              fp(fp), c(c) {}

        template <int d>
        Vec<double, d, true> inFp(int i) {
            return Vec<double, d, true>(&fp[opData[i]]);
        }
        char* inStr(int i) { return c[opData[i]]; }
        int nargs() const { return _nargs; }

        /// Return a vector handle which is easier to assign to
        template <int d>
        Vec<double, d, true> outFpHandle() {
            return Vec<double, d, true>(&outFp);
        }

        double& outFp;
        char*& outStr;
        const ExprFuncNode::Data* data;
        const char* varBlock;

      private:
        int _nargs;
        const int* opData;
        double* fp;
        char** c;
        // std::stack<int>& callStack;
    };

    virtual int buildInterpreter(const ExprFuncNode* node, Interpreter* interpreter) const;

    virtual ExprType prep(ExprFuncNode* node, bool scalarWanted, ExprVarEnvBuilder& envBuilder) const = 0;
    virtual ExprFuncNode::Data* evalConstant(const ExprFuncNode*, ArgHandle) const { return nullptr; }
    virtual void eval(ArgHandle args) = 0;

    static ExprType genericPrep(ExprFuncNode* node,
                                bool scalarWanted,
                                ExprVarEnvBuilder& env,
                                const ExprFuncDeclaration& decl);

  private:
    static int EvalOp(const int* opData, double* fp, char** c, std::vector<int>& callStack);
};

template <typename FunctionCodeStorage>
class ExprFuncClosure : public ExprFuncSimple {
  public:
    ExprFuncClosure() : ExprFuncSimple(true), _callable(nullptr) {}
    explicit ExprFuncClosure(FunctionCodeStorage callable_) : ExprFuncSimple(true), _callable(callable_) {}

    virtual ExprType prep(ExprFuncNode* node, bool scalarWanted, ExprVarEnvBuilder& envBuilder) const override {
        assert(false);
        return ExprType().Error();
    }

    virtual ExprFuncNode::Data* evalConstant(const ExprFuncNode* node, ArgHandle args) const override {
        return nullptr;
    }

    virtual inline void eval(ArgHandle args) override {
        assert(_callable);
        _callable(args);
    }

  private:
    FunctionCodeStorage _callable;
};

class ExprFuncLocal : public ExprFuncX {
    ExprFuncLocal() : ExprFuncX(true) {}

    /** prep the expression by doing all type checking argument checking, etc. */
    virtual ExprType prep(ExprFuncNode* node, bool scalarWanted, ExprVarEnvBuilder& envBuilder) const;
    //! Build an interpreter to evaluate the expression
    virtual int buildInterpreter(const ExprFuncNode* node, Interpreter* interpreter) const;
};
}

#endif
