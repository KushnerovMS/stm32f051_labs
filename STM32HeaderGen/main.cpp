#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <strstream>

// This badlooking program is used to generate defines for registers of STM32

struct Bit
{
    std::string     name;
    int             offset;
    int             length;
    std::string     comment;
};

struct Reg 
{
    std::string name;
    void*       pointer;
    int         offset_mul;
    std::string comment;
    int         bitsc;
    Bit         bits[32];
};


int main (int carg, char ** varg)
{
    if (carg <= 1)
    {
        std::cout << "Enter in file to generate\n";
        exit(1);
    }

    std::list<Reg> reg_list;

    std::string controller_name;


    std::ifstream in_file_stream;
    in_file_stream.open(varg[1]);

    in_file_stream >> controller_name;

    std::string line;
    std::getline(in_file_stream, line);
    while (in_file_stream.good())
    {
        if (line.length() < 2)
        {
            std::getline(in_file_stream, line);
            continue;
        }
        std::strstream sline;
        sline << line;

        //std::cout << line << std::endl;


        Reg new_reg;
        sline >> new_reg.name;
        sline >> new_reg.pointer;
        sline >> new_reg.offset_mul;
        if (sline.fail())
        {
            new_reg.offset_mul = 1;
            sline.clear();
        }
        getline(sline, new_reg.comment);

        new_reg.bitsc = 0;
        int i = 0;
        for (i = 0; i < 32; i ++)
        {
            if (in_file_stream.eof())
                break;
            std::getline(in_file_stream, line);
            //std::cout << line << std::endl;
            if (line.length() < 2)
                continue;
            else if (line[0] != ' ' && line[0] != '\t')
                break;
            std::strstream sline;
            sline << line;

            sline >> new_reg.bits[i].name;
            sline >> new_reg.bits[i].offset;
            sline >> new_reg.bits[i].length;
            std::getline(sline, new_reg.bits[i].comment);

            new_reg.bitsc ++;
        }
        //std::cout << i << ' ' << new_reg.bitsc << std::endl;


        reg_list.push_back(new_reg);
    }

    //std::cout << std::endl;

    in_file_stream.close();
    

    /*
    for (auto& item : reg_list)
    {
        //std::cout << item.name << " : " << item.pointer << " : " << item.offset_mul << " : " << item.comment << std::endl;
        for (int i = 0; i < item.bitsc; i++)
            std::cout << item.bits[i].name << " : "
                      << item.bits[i].offset << " : " 
                      << item.bits[i].length << " : "
                      << item.bits[i].comment << std::endl;
    }
    */

    std::ofstream out_file_stream;
    out_file_stream.open(controller_name + ".h");

    for (auto& item : reg_list)
    {
        out_file_stream << "#define REG_" << item.name << " (volatile uint32_t*)(uintptr_t)" << item.pointer << 'U';
        if (item.comment.length() > 0)
            out_file_stream << " // " << item.comment;
        out_file_stream << std::endl;

        for (int i = 0; i < item.bitsc; i++)
        {
            out_file_stream << "#define " << item.name << '_' << item.bits[i].name << ' ' << item.bits[i].offset << 'U';
            if (item.bits[i].comment.length() > 0)
                out_file_stream << " // " << item.bits[i].comment;
            out_file_stream << std::endl;
        }
        out_file_stream << std::endl;
    }

    out_file_stream << "\n/////////////////////////////////////////////////////////////////////////////////////////////////////\n\n";

    out_file_stream << "#define SET_BIT(REG, BIT)   *(REG) |= 1U << (BIT)\n"
                       "#define CLEAR_BIT(REG, BIT) *(REG) &= ~(1U << (BIT))\n"
                       "#define READ_BIT(REG, BIT)  ((*(REG) >> (BIT)) & 1U)\n"
                       "\n"
                       "#define SET_BITS(REG, SBIT, VAL)    *(REG) |= (VAL) << (SBIT)\n"
                       "#define RESET_BITS(REG, SBIT, MASK, VAL) *(REG) = ((*(REG) & ~((MASK) << (SBIT))) | ((VAL) << (SBIT)))\n"
                       "#define READ_BITS(REG, SBIT, MASK)  ((*(REG) >> (SBIT)) & (MASK))\n";

    out_file_stream << "\n/////////////////////////////////////////////////////////////////////////////////////////////////////\n\n";

    for (auto& item : reg_list)
    {
        if (item.offset_mul == 1)
        {
            out_file_stream << "#define SET_"<<item.name<<"_BIT(BIT) SET_BIT(REG_"<< item.name<<", BIT)\n";
            out_file_stream << "#define SET_"<<item.name<<"_BITS(SBIT, VAL) SET_BITS(REG_"<< item.name<<", SBIT, VAL)\n";
            out_file_stream << "#define CLEAR_"<<item.name<<"_BIT(BIT) CLEAR_BIT(REG_"<< item.name<<", BIT)\n";
            out_file_stream << "#define READ_"<<item.name<<"_BIT(BIT) READ_BIT(REG_"<< item.name<<", BIT)\n";
        }
        else
        {
            std::string mask = "0b";
            for (int k = 0; k < item.offset_mul; k ++)
                mask.push_back('1');
            mask.push_back('U');

            out_file_stream << "#define SET_"<<item.name<<"_BITS(BIT, VAL) SET_BITS(REG_"<< item.name<<", "<<item.offset_mul<<"U*BIT, VAL)\n";
            out_file_stream << "#define RESET_"<<item.name<<"_BITS(BIT, VAL) RESET_BITS(REG_"<< item.name<<", "<<item.offset_mul<<"U*BIT, "<<mask<<", VAL)\n";
            out_file_stream << "#define READ_"<<item.name<<"_BITS(BIT) READ_BITS(REG_"<< item.name<<", "<<item.offset_mul<<"U*BIT, "<<mask<<")\n";
        }

        out_file_stream << std::endl;

        for (int i = 0; i < item.bitsc; i ++)
        {
            Bit bit = item.bits[i];
            //std::cout << bit.length << std::endl;
            if (bit.length == 1)
            {
                out_file_stream << "#define SET_"<<item.name<<'_'<<bit.name<<"() SET_BIT(REG_"<<item.name<<", "<<item.name<<'_'<<bit.name<<")\n";
                out_file_stream << "#define CLEAR_"<<item.name<<'_'<<bit.name<<"() CLEAR_BIT(REG_"<<item.name<<", "<<item.name<<'_'<<bit.name<<")\n";
                out_file_stream << "#define READ_"<<item.name<<'_'<<bit.name<<"() READ_BIT(REG_"<<item.name<<", "<<item.name<<'_'<<bit.name<<")\n";
            }
            else
            {
                std::string mask = "0b";
                for (int k = 0; k < bit.length; k ++)
                    mask.push_back('1');
                mask.push_back('U');

                out_file_stream << "#define SET_"<<item.name<<'_'<<bit.name<<"(VAL) SET_BITS(REG_"<< item.name<<", "<<item.name<<'_'<<bit.name<<", VAL)\n";
                out_file_stream << "#define RESET_"<<item.name<<'_'<<bit.name<<"(VAL) RESET_BITS(REG_"<< item.name<<", "<<item.name<<'_'<<bit.name<<", "<<mask<<", VAL)\n";
                out_file_stream << "#define READ_"<<item.name<<'_'<<bit.name<<"() READ_BITS(REG_"<< item.name<<", "<<item.name<<'_'<<bit.name<<", "<<mask<<")\n";
            }

            out_file_stream << std::endl;
        }

        out_file_stream << std::endl;
    }


    out_file_stream.close();

    return 0;

}
