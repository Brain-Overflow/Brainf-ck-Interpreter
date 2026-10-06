#include <iostream>
#include <fstream>
#include <format>
#include <filesystem>
#include <string>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <utility>
#include <stack>

class Interpreter {
    private:
        std::string instructions;
        std::size_t instruction_pointer = 0;
        std::vector<std::uint8_t> data_cells;
        std::size_t data_pointer = 0;
        static constexpr std::size_t initial_number_of_data_cells = 30000;
        std::vector<std::size_t> jump_table;

        void execute_current_instruction() {
            const char current_instruction = instructions[instruction_pointer];

            switch(current_instruction)
            {
                case '>':
                    ++data_pointer;

                    if (data_pointer == data_cells.size()) {
                        data_cells.push_back(0);
                    } 

                    break;
                case '<':
                    if (data_pointer > 0) {
                        --data_pointer;
                    }
                    else {
                        throw std::runtime_error("Error: Data pointer cannot be less than zero!");
                    }  

                    break;
                case '+':
                    ++data_cells[data_pointer];

                    break;
                case '-':
                    --data_cells[data_pointer];

                    break;
                case '.':
                    std::cout.put(static_cast<char>(data_cells[data_pointer]));

                    break;
                case ',': {
                    int input_character = std::cin.get();

                    // Leave the data cell unchanged when the input character is EOF.
                    if (input_character != EOF) {
                        data_cells[data_pointer] = static_cast<std::uint8_t>(input_character);
                    }

                    break;
                }
                case '[': {
                    if (data_cells[data_pointer] == 0) {
                        instruction_pointer = jump_table[instruction_pointer];
                    }

                    break;
                }
                case ']': {
                    if (data_cells[data_pointer] != 0) {
                        instruction_pointer = jump_table[instruction_pointer];
                    }

                    break;
                }
                default:
                    // Non-command characters are comments.
                    break;
            }

            ++instruction_pointer;
        }

        bool is_finished() const {
            return instruction_pointer >= instructions.size();
        }

        void build_jump_table() {
            std::stack<std::size_t> jump_stack;

            std::size_t temporary_instruction_pointer = 0;

            while (temporary_instruction_pointer < instructions.size()) {
                char current_instruction = instructions[temporary_instruction_pointer];

                if (current_instruction == '[') {
                    jump_stack.push(temporary_instruction_pointer);
                }
                else if (current_instruction == ']') {
                    if (!jump_stack.empty()) {
                        const std::size_t opening_bracket_instruction_pointer = jump_stack.top();
                        jump_table[opening_bracket_instruction_pointer] = temporary_instruction_pointer;
                        jump_table[temporary_instruction_pointer] = opening_bracket_instruction_pointer;
                        jump_stack.pop();
                    } else {
                        throw std::runtime_error(std::format("Error: Found a ']' at {} without a corresponding '['!", temporary_instruction_pointer));
                    }
                }

                ++temporary_instruction_pointer;
            }

            if (!jump_stack.empty()) {
                throw std::runtime_error(std::format("Error: Found a '[' at {} without a corresponding ']'!", jump_stack.top()));
            }
        }

    public:
        explicit Interpreter(std::string instructions_) : instructions(std::move(instructions_)), data_cells(initial_number_of_data_cells), jump_table(instructions.size()) {
            build_jump_table();
        }

        // Return the data cell at the index or 0 for every data cell past the end of the tape.
        std::uint8_t retrieve_data_cell(std::size_t index) const {
            std::uint8_t return_value = 0;

            if (index < data_cells.size()) {
                return_value = data_cells[index];
            }

            return return_value;
        }      
        
        void execute_instructions() {
            while (!is_finished()) {
                execute_current_instruction();
            }
        }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Error: Invalid arguments received!\nUsage: ./brainfuck <filepath>\n";
        return 1;
    }

    std::ifstream program_file(argv[1]);

    if (!program_file) {
        std::cerr << std::format("Error: Could not open \"{}\"!\n", argv[1]);
        return 1;
    }

    try {
        const auto file_size = std::filesystem::file_size(argv[1]);

        if (!file_size) {
            std::cerr << "Error: File does not contain a program!\n";
            return 1;
        }

        std::string file_contents(file_size, '\0');

        program_file.read(file_contents.data(), file_size);

        if (!program_file) {
            std::cerr << std::format("Error: Could not read the entirety of \"{}\"!\nError: Read {} bytes, expected {} bytes!\n", argv[1], program_file.gcount(), file_size);
            return 1;
        }

        //std::cout << std::format("File contents: {}\n", file_contents);

        Interpreter interpreter(std::move(file_contents));

        interpreter.execute_instructions();
    }
    catch(const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}