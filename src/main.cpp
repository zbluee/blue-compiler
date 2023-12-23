#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "./include/codeGenerator.h"
#include "./include/scanner.h"

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Error : Invalid Usage blue <filename>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream buf;
        std::ifstream input(argv[1]);
        if (!input.is_open())
        {
            std::cerr << "Error: unable to open a file for writing." << std::endl;
            return EXIT_FAILURE;
        }
        buf << input.rdbuf();
        contents = buf.str();
    }
    std::cout << contents << std::endl;
    Scanner sc(contents);
    std::vector<Token> tokens = sc.tokenize();

    Parser parser(tokens);
    std::optional<node::Prog> prog = parser.parseProg();

    if (!prog.has_value())
    {
        std::cerr << "Error : Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    CodeGenerator generator(prog.value());

    {
        std::ofstream write("../out.asm");
        write << generator.genProg();
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}