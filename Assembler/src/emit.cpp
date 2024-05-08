#include "../codegen/emit.hpp"

masm::emit::Emit::Emit(masm::codegen::Codegen &codegen)
{
    instructions = codegen.get_instructions();
    data = codegen.get_data();
    entry = codegen.get_entry_addr();
    str_len = codegen.get_str_len();
}

void masm::emit::Emit::emit()
{
    // first open it up
    std::fstream out(out_file_name, std::ios::out | std::ios::binary);

    if (!out.is_open())
    {
        std::cerr << "Failed to generate binary; Terminating." << std::endl;
        exit(EXIT_FAILURE);
    }
    uint8_t header[32] = {'M', 'I', 'N'};
    // now the entry point
    header[3] = (entry >> 32) & 255;
    header[4] = (entry >> 24) & 255;
    header[5] = (entry >> 16) & 255;
    header[6] = (entry >> 8) & 255;
    header[7] = (entry) & 255;

    // next the instruction bytes
    size_t inst_len = instructions.size() * 8;
    header[8] = (inst_len >> 56) & 255;
    header[9] = (inst_len >> 48) & 255;
    header[10] = (inst_len >> 40) & 255;
    header[11] = (inst_len >> 32) & 255;
    header[12] = (inst_len >> 24) & 255;
    header[13] = (inst_len >> 16) & 255;
    header[14] = (inst_len >> 8) & 255;
    header[15] = (inst_len) & 255;

    size_t data_len = data.size() - str_len;
    header[16] = (data_len >> 56) & 255;
    header[17] = (data_len >> 48) & 255;
    header[18] = (data_len >> 40) & 255;
    header[19] = (data_len >> 32) & 255;
    header[20] = (data_len >> 24) & 255;
    header[21] = (data_len >> 16) & 255;
    header[22] = (data_len >> 8) & 255;
    header[23] = (data_len) & 255;

    header[24] = (str_len >> 56) & 255;
    header[25] = (str_len >> 48) & 255;
    header[26] = (str_len >> 40) & 255;
    header[27] = (str_len >> 32) & 255;
    header[28] = (str_len >> 24) & 255;
    header[29] = (str_len >> 16) & 255;
    header[30] = (str_len >> 8) & 255;
    header[31] = (str_len) & 255;

    // for now we will leave the remaining 8 bytes as is since we are yet to implement strings
    for (auto h : header)
    {
        out << h;
    }

    for (auto inst : instructions)
    {
        out << inst.bytes.b8;
        out << inst.bytes.b7;
        out << inst.bytes.b6;
        out << inst.bytes.b5;
        out << inst.bytes.b4;
        out << inst.bytes.b3;
        out << inst.bytes.b2;
        out << inst.bytes.b1;
    }

    for (auto d : data)
    {
        out << d;
    }

    out.close();
}