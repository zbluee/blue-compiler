#pragma once
#include "./parser.h"
#include <unordered_map>

class CodeGenerator
{
private:
    const node::Prog m_Prog;
    mutable std::stringstream m_Output;
    size_t m_StackPtr; // to keep track where the stack ptr will be at compile time

    struct Variable
    {
        // for the variable, it's posistion in the stack need to known
        size_t stackPtr;
    };

    std::unordered_map<std::string, Variable> m_Variables{}; // to keep track the variables, can't declare same variable twice

    void push(const std::string &reg)
    {
        m_Output << "    PUSH " << reg << "\n";
        m_StackPtr++;
    }

    void pop(const std::string &reg)
    {
        m_Output << "    POP " << reg << "\n";
        m_StackPtr--;
    }

public:
    inline CodeGenerator(const node::Prog& prog) : m_Prog(prog), m_StackPtr(0) {}

    void genTerm(const node::Term *term)
    {
        struct TermVisitor
        {
            CodeGenerator *generator;
            void operator()(const node::TermIntLit *termIntLit) const
            {
                // push in int lit value in the stack
                generator->m_Output << "    MOV rax, " << termIntLit->int_literal.value.value() << "\n";
                generator->push("rax");
            }
            void operator()(const node::TermIdent *termIdent) const
            {
                // extracting out the value of the varialbe and we need to put copy of it on top of the stack
                // first check if the variable is declared
                if (!generator->m_Variables.contains(termIdent->ident.value.value()))
                {
                    std::cerr << "Error : Undeclared Identifier : " << termIdent->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // get the value from the stack using stack_ptr and push it to on the top of the stack
                // offset from stack_ptr
                const auto &val = generator->m_Variables.at(termIdent->ident.value.value());
                std::stringstream offset;
                // the size of the data,(pushed) should be specified, b/c using 64 bit, it's denoted as QWORD
                // the offset is in bytes and the stack_ptr is using one for 64 bits so, to overcome this multiply by 8 simply to access the next index element in assebmly arr[curr + 8]
                offset << "QWORD [rsp + " << (generator->m_StackPtr - val.stackPtr - 1) * 8 << "]\n";
                generator->push(offset.str());
            }
            void operator()(const node::TermParenthesis* termParenthesis) const{
                generator->genExpr(termParenthesis->expr);
            }
        };
        TermVisitor visitor {.generator = this};
        std::visit(visitor, term->variant);
    }

    void genExprs(const node::Exprs *exprs)
    {
        struct ExprsVisitor
        {
            CodeGenerator *generator;
            void operator()(const node::ExprsAdd *add) const
            {
                generator->genExpr(add->rhs);
                generator->genExpr(add->lhs);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_Output << "    ADD rax, rbx\n";
                generator->push("rax");
            }
            void operator()(const node::ExprsSub *sub) const
            {
                generator->genExpr(sub->rhs);
                generator->genExpr(sub->lhs);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_Output << "    SUB rax, rbx\n";
                generator->push("rax");
            }
            void operator()(const node::ExprsMul *mul) const
            {
                generator->genExpr(mul->rhs);
                generator->genExpr(mul->lhs);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_Output << "    MUL rbx\n";
                generator->push("rax");
            }

            void operator()(const node::ExprsDiv *div) const
            {
                generator->genExpr(div->rhs);
                generator->genExpr(div->lhs);
                generator->pop("rax");
                generator->pop("rbx");
                generator->m_Output << "    DIV rbx\n";
                generator->push("rax");
            }
        };

        ExprsVisitor visitor {.generator = this};
        std::visit(visitor, exprs->variant);
    }

    void genExpr(const node::Expr *expr)
    {

        struct ExprVisitor
        {
            CodeGenerator *generator;
            void operator()(const node::Term *term) const
            {
                generator->genTerm(term);
            }

            void operator()(const node::Exprs *exprs) const
            {
                generator->genExprs(exprs);
            }
        };

        ExprVisitor visitor {.generator = this};
        // if expr.variant is intlit it calls the overloaded operator with ExprIntLit params, or if it's ExprIdent it calls the overloaded operator with ExprIdent
        std::visit(visitor, expr->variant);
    }

    void genStatement(const node::Statement *statement)
    {
        struct StatementVisitor
        {
            CodeGenerator *generator;
            void operator()(const node::StatementExit *statementExit) const
            {
                generator->genExpr(statementExit->expr);
                generator->m_Output << "    MOV rax, 60\n";
                generator->pop("rdi");
                generator->m_Output << "    syscall\n";
            }
            void operator()(const node::StatementLet *statementLet) const
            {
                // encounter with let statement, first need to check to make sure that there is not a variable declared with that name
                if (generator->m_Variables.contains(statementLet->ident.value.value()))
                {
                    std::cerr << "Error :  Redeclaration of variable : " << statementLet->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // copy the value of the stack at stack_ptr and push it on the top of the stack and then
                // when exit is called simply pop it from the stack.
                generator->m_Variables.insert({statementLet->ident.value.value(), Variable{.stackPtr = generator->m_StackPtr}});
                // evaluate the expression
                generator->genExpr(statementLet->expr); // now the value of the expression is on the top of the stack
            }
        };

        StatementVisitor visitor {.generator = this};
        // if statement.variant is exit it calls the overloaded operator with StatementExit params, or if it's StatementLet it calls the overloaded operator with StatementLet
        std::visit(visitor, statement->variant);
    }

    std::string genProg()
    {

        m_Output << "global _start\n_start:\n";

        for (const node::Statement *statement : m_Prog.statements)
            genStatement(statement);

        // default exit with 0, if there is no exit in the code, it will call the exit syscall by default
        m_Output << "    MOV rax, 60\n"; // MOV NR value for the exit system call to rax register
        m_Output << "    MOV rdi, 0\n";
        m_Output << "    syscall\n";
        return m_Output.str();
    }
};
