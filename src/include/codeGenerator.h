#pragma once
#include "./parser.h"
#include <map>

class CodeGenerator
{
private:
    const node::Prog m_Prog;
    mutable std::stringstream m_Output;
    size_t m_CountLabel;
    size_t m_StackPtr; // to keep track where the stack ptr will be at compile time

    struct Variable
    {
        std::string name;
        size_t stackPtr; // for the variable, it's posistion in the stack need to known
    };

    std::vector<Variable> m_Variables{}; // to keep track the variables, can't declare same variable twice
    std::vector<size_t> m_Scopes{};      // indices to the variables

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

    void beginScope()
    {
        m_Scopes.push_back(m_Variables.size());
    }

    void endScope()
    {
        const size_t popCount = m_Variables.size() - m_Scopes.back();
        // each of the variable is a 8 bytes b/c we r using 64 bit int and since the stack grows downward in memory by increasing the value of rsp, moving the sp upward in memory, which has the effect of "popping" elements off the stack.
        m_Output << "    ADD rsp, " << popCount * 8 << "\n";
        m_StackPtr -= popCount;

        for (size_t i = 0; i < popCount; i++)
            m_Variables.pop_back();

        m_Scopes.pop_back();
    }

    std::string createLabel()
    {
        return "label" + std::to_string(m_CountLabel++);
    }

public:
    inline CodeGenerator(const node::Prog &prog) : m_Prog(prog), m_StackPtr(0), m_CountLabel(0) {}

    void genTerm(const node::Term *term)
    {
        struct TermVisitor
        {
            CodeGenerator &generator;
            void operator()(const node::TermIntLit *termIntLit) const
            {
                // push in int lit value in the stack
                generator.m_Output << "    MOV rax, " << termIntLit->int_literal.value.value() << "\n";
                generator.push("rax");
            }
            void operator()(const node::TermIdent *termIdent) const
            {
                const auto itr = std::find_if(generator.m_Variables.cbegin(), generator.m_Variables.cend(), [&](const Variable &var)
                                              { return var.name == termIdent->ident.value.value(); });
                // extracting out the value of the varialbe and we need to put copy of it on top of the stack
                // first check if the variable is declared
                if (itr == generator.m_Variables.cend())
                {
                    std::cerr << "Error : Undeclared Identifier : " << termIdent->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // get the value from the stack using stack_ptr and push it to on the top of the stack
                // offset from stack_ptr
                std::stringstream offset;
                // the size of the data,(pushed) should be specified, b/c using 64 bit, it's denoted as QWORD
                // the offset is in bytes and the stack_ptr is using one for 64 bits so, to overcome this multiply by 8 simply to access the next index element in assebmly arr[curr + 8]
                offset << "QWORD [rsp + " << (generator.m_StackPtr - (*itr).stackPtr - 1) * 8 << "]";
                generator.push(offset.str());
            }
            void operator()(const node::TermParenthesis *termParenthesis) const
            {
                generator.genExpr(termParenthesis->expr);
            }
        };
        TermVisitor visitor{.generator = *this};
        std::visit(visitor, term->variant);
    }

    void genExprs(const node::Exprs *exprs)
    {
        struct ExprsVisitor
        {
            CodeGenerator &generator;
            void operator()(const node::ExprsAdd *add) const
            {
                generator.genExpr(add->rhs);
                generator.genExpr(add->lhs);
                generator.pop("rax");
                generator.pop("rbx");
                generator.m_Output << "    ADD rax, rbx\n";
                generator.push("rax");
            }
            void operator()(const node::ExprsSub *sub) const
            {
                generator.genExpr(sub->rhs);
                generator.genExpr(sub->lhs);
                generator.pop("rax");
                generator.pop("rbx");
                generator.m_Output << "    SUB rax, rbx\n";
                generator.push("rax");
            }
            void operator()(const node::ExprsMul *mul) const
            {
                generator.genExpr(mul->rhs);
                generator.genExpr(mul->lhs);
                generator.pop("rax");
                generator.pop("rbx");
                generator.m_Output << "    MUL rbx\n";
                generator.push("rax");
            }

            void operator()(const node::ExprsDiv *div) const
            {
                generator.genExpr(div->rhs);
                generator.genExpr(div->lhs);
                generator.pop("rax");
                generator.pop("rbx");
                generator.m_Output << "    DIV rbx\n";
                generator.push("rax");
            }
        };

        ExprsVisitor visitor{.generator = *this};
        std::visit(visitor, exprs->variant);
    }

    void genExpr(const node::Expr *expr)
    {

        struct ExprVisitor
        {
            CodeGenerator &generator;
            void operator()(const node::Term *term) const
            {
                generator.genTerm(term);
            }

            void operator()(const node::Exprs *exprs) const
            {
                generator.genExprs(exprs);
            }
        };

        ExprVisitor visitor{.generator = *this};
        // if expr.variant is intlit it calls the overloaded operator with ExprIntLit params, or if it's ExprIdent it calls the overloaded operator with ExprIdent
        std::visit(visitor, expr->variant);
    }

    void genScope(const node::Scope *scope)
    {
        beginScope();
        for (const node::Statement *statement : scope->statements)
            genStatement(statement);

        endScope();
    }

    void genConditionalBr(const node::ConditionalBranch *conditionalBr, const std::string &endLabel)
    {

        struct ConditionalBranchVisitor
        {
            CodeGenerator &generator;
            const std::string &endLabel;

            void operator()(const node::ConditionalBranchElif *conditionalBrElif) const
            {
                generator.genExpr(conditionalBrElif->expr);
                generator.pop("rax");
                const std::string label = generator.createLabel();
                generator.m_Output << "    TEST rax, rax\n";
                generator.m_Output << "    JZ " << label << "\n";
                generator.genScope(conditionalBrElif->scope);
                // as soon as one of the elif statement is true, jump the rest
                generator.m_Output << "    JMP " << endLabel << "\n";

                if (conditionalBrElif->conditionalBr.has_value())
                {
                    generator.m_Output << label << ":\n";
                    generator.genConditionalBr(conditionalBrElif->conditionalBr.value(), endLabel);
                }
            }
            void operator()(const node::ConditionalBranchElse *conditionalBrElse) const
            {
                generator.genScope(conditionalBrElse->scope);
            }
        };

        ConditionalBranchVisitor visitor{.generator = *this, .endLabel = endLabel};
        std::visit(visitor, conditionalBr->variant);
    }
    void genStatement(const node::Statement *statement)
    {
        struct StatementVisitor
        {
            CodeGenerator &generator;
            void operator()(const node::StatementExit *statementExit) const
            {
                generator.genExpr(statementExit->expr);
                generator.m_Output << "    MOV rax, 60\n";
                generator.pop("rdi");
                generator.m_Output << "    syscall\n";
            }
            void operator()(const node::StatementLet *statementLet) const
            {
                const auto it = std::find_if(generator.m_Variables.cbegin(), generator.m_Variables.cend(), [&](const Variable &var)
                                             { return var.name == statementLet->ident.value.value(); });
                // encounter with let statement, first need to check to make sure that there is not a variable declared with that name
                if (it != generator.m_Variables.cend())
                {
                    std::cerr << "Error :  Redeclaration of variable : " << statementLet->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // copy the value of the stack at stack_ptr and push it on the top of the stack and then
                // when exit is called simply pop it from the stack.
                generator.m_Variables.push_back({.name = statementLet->ident.value.value(), .stackPtr = generator.m_StackPtr});
                // evaluate the expression
                generator.genExpr(statementLet->expr); // now the value of the expression is on the top of the stack
            }
            void operator()(const node::Scope *scope) const
            {
                generator.genScope(scope);
            }
            void operator()(const node::StatementIf *statementIf) const
            {
                // evaluate the expression first
                generator.genExpr(statementIf->expr); // this will put the result of the expression at the stuck top
                generator.pop("rax");                 // pop back in to the rax, if it's 0 jump, else go to the label
                const std::string label = generator.createLabel();
                generator.m_Output << "    TEST rax, rax\n";
                generator.m_Output << "    JZ " << label << "\n";
                generator.genScope(statementIf->scope);
                generator.m_Output << label << ":\n";

                if (statementIf->conditionalBr.has_value())
                {
                    const std::string endLabel = generator.createLabel();
                    generator.genConditionalBr(statementIf->conditionalBr.value(), endLabel);
                    generator.m_Output << endLabel << ":\n";
                }
            }
        };

        StatementVisitor visitor{.generator = *this};
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
