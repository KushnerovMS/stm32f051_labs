#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <strstream>

// This badlooking program is used to generate defines for registers of STM32

struct CustVal
{
    std::string     name;
    std::string     val;
    std::string     comment;
};

struct Bit
{
    std::string         name;
    int                 offset;
    int                 length;
    std::string         comment;
    std::list<CustVal>  custom_val;
};

struct Reg 
{
    std::string         name;
    void*               pointer;
    int                 offset_mul;
    std::string         comment;
    std::list<Bit>      bits;
    std::list<CustVal>  custom_val;
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

    enum ParsingState
    {
        REGISTER,
        BIT,
        CUST_VAL
    } state;
    std::string line;
    state = REGISTER;
    std::list<Bit>* cur_bits = nullptr;
    std::list<CustVal>* cur_cust_val_list = nullptr;
    while (in_file_stream.good())
    {
        std::getline(in_file_stream, line);
        if (line.length() < 2)
            continue;

        if (line[0] != ' ' && line[0] != '\t' && line[0] != ':')
        {
//            std::cout << "Register: " << line << std::endl;

            state = REGISTER;
            std::strstream sline;
            sline << line;

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

            reg_list.push_back(new_reg);

            cur_bits = & reg_list.back().bits;
            cur_cust_val_list = & reg_list.back().custom_val;
        }
        else
        {
            int skip = 0;
            while (line[skip] == ' ' || line[skip] == '\t') skip ++;

            if (line[skip] == ':')
            {
                state = CUST_VAL;
                std::strstream sline;
                sline << line.substr(skip + 1);

 //               std::cout << "Custom value: " << line.substr(skip + 1) << std::endl;

                CustVal new_cust_val;
                sline >> new_cust_val.val;
                sline >> new_cust_val.name;
                getline(sline, new_cust_val.comment);

                cur_cust_val_list -> push_back(new_cust_val);
            }
            else
            {
                state = BIT;
                std::strstream sline;
                sline << line.substr(skip);

  //              std::cout << "Bit: " << line.substr(skip) << std::endl;

                Bit new_bit;
                sline >> new_bit.name;
                sline >> new_bit.offset;
                sline >> new_bit.length;
                getline(sline, new_bit.comment);

                cur_bits -> push_back(new_bit);

                cur_cust_val_list = & cur_bits -> back().custom_val;
            }
        }
    }

    for (auto& reg : reg_list)
    {
        size_t pos = reg.name.find('x');
        if (pos != std::string::npos)
        {
//            reg.name.erase(pos);

            for (char i = 0; i <= 'F' - 'A'; ++ i)
            {
                Reg newReg = reg;
                newReg.name[pos] = 'A' + i;
                newReg.pointer += i * 0x400U;
                reg_list.push_back(newReg);
            }
        }
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

    for (auto& reg : reg_list)
    {
        if (reg.name.find('x') == std::string::npos)
            out_file_stream << "#define REG_" << reg.name << " (volatile uint32_t*)(uintptr_t)" << reg.pointer << 'U';
        else
            out_file_stream << "#define REG_" << reg.name << "(X) (volatile uint32_t*)(uintptr_t)(" << reg.pointer << "U + 0x400U * (unsigned)(X - 'A'))";


        if (reg.comment.length() > 0)
            out_file_stream << " // " << reg.comment;
        out_file_stream << std::endl;

        for (auto& cust_val : reg.custom_val)
        {
            if (cust_val.val.find("VAL") == std::string::npos)
                out_file_stream << "#define "<<reg.name<<'_'<<cust_val.name<<" 0b"<<cust_val.val<<'U';
            else
                out_file_stream << "#define "<<reg.name<<'_'<<cust_val.name<<"(VAL) "<<cust_val.val;

            if (cust_val.comment.length() > 0)
                out_file_stream << " // " << cust_val.comment;
            out_file_stream << std::endl;
        }

        for (auto& bit : reg.bits)
        {
            out_file_stream << "#define " << reg.name << '_' << bit.name << ' ' << bit.offset << 'U';
            if (bit.comment.length() > 0)
                out_file_stream << " // " << bit.comment;
            out_file_stream << std::endl;

            for (auto& cust_val : bit.custom_val)
            {
                if (cust_val.val.find("VAL") == std::string::npos)
                    out_file_stream << "#define "<<reg.name<<'_'<<bit.name<<'_'<<cust_val.name<<" 0b"<<cust_val.val<<'U';
                else
                    out_file_stream << "#define "<<reg.name<<'_'<<bit.name<<'_'<<cust_val.name<<"(VAL) "<<cust_val.val;

                if (cust_val.comment.length() > 0)
                    out_file_stream << " // " << cust_val.comment;
                out_file_stream << std::endl;
            }
        }
        out_file_stream << std::endl;
    }

    out_file_stream << "\n/////////////////////////////////////////////////////////////////////////////////////////////////////\n\n";

    out_file_stream << "#define SET_BITS(REG, SBIT, VAL)         *(REG) |= (VAL) << (SBIT)\n"
                       "#define RESET_BITS(REG, SBIT, MASK, VAL) *(REG) = ((*(REG) & ~((MASK) << (SBIT))) | ((VAL) << (SBIT)))\n"
                       "#define READ_BITS(REG, SBIT, MASK)       ((*(REG) >> (SBIT)) & (MASK))\n"
                       "\n"
                       "#define SET_BIT(REG, BIT)        *(REG) |= 1U << (BIT)\n"
                       "#define CLEAR_BIT(REG, BIT)      *(REG) &= ~(1U << (BIT))\n"
                       "#define RESET_BIT(REG, BIT, VAL) RESET_BITS(REG, BIT, 0b1U, VAL)\n"
                       "#define READ_BIT(REG, BIT)       READ_BITS(REG, BIT, 0b1U)\n";

    out_file_stream << "\n/////////////////////////////////////////////////////////////////////////////////////////////////////\n\n";

    for (auto& reg : reg_list)
    {
        if (reg.name.find('x') == std::string::npos)
        {
            std::string regMask;
            if (reg.offset_mul == 1)
            {
                regMask = "0b1U";
                out_file_stream << "#define SET_"<<reg.name<<"_BIT(BIT) SET_BIT(REG_"<<reg.name<<", BIT)\n";
                out_file_stream << "#define CLEAR_"<<reg.name<<"_BIT(BIT) CLEAR_BIT(REG_"<<reg.name<<", BIT)\n";
                out_file_stream << "#define RESET_"<<reg.name<<"_BIT(BIT, VAL) RESET_BIT(REG_"<<reg.name<<", BIT, VAL)\n";
                out_file_stream << "#define READ_"<<reg.name<<"_BIT(BIT) READ_BIT(REG_"<<reg.name<<", BIT)\n";
            }
            else
            {
                regMask = "0b";
                for (int k = 0; k < reg.offset_mul; k ++)
                    regMask.push_back('1');
                regMask.push_back('U');

                out_file_stream << "#define SET_"<<reg.name<<"_BITS(BIT, VAL) SET_BITS(REG_"<<reg.name<<", "<<reg.offset_mul<<"U*BIT, VAL)\n";
                out_file_stream << "#define RESET_"<<reg.name<<"_BITS(BIT, VAL) RESET_BITS(REG_"<<reg.name<<", "<<reg.offset_mul<<"U*BIT, "<<regMask<<", VAL)\n";
                out_file_stream << "#define READ_"<<reg.name<<"_BITS(BIT) READ_BITS(REG_"<<reg.name<<", "<<reg.offset_mul<<"U*BIT, "<<regMask<<")\n";
            }

            for (auto& custVal : reg.custom_val) 
                if (custVal.val.find("VAL") == std::string::npos)
                    out_file_stream << "#define SET_"<<reg.name<<'_'<<custVal.name<<"(BIT) RESET_BITS(REG_"<<reg.name<<", "<<reg.offset_mul<<"U*BIT, "<<regMask<<", "<<reg.name<<'_'<<custVal.name<<")\n"; 
                else
                    out_file_stream << "#define SET_"<<reg.name<<'_'<<custVal.name<<"(BIT, VAL) RESET_BITS(REG_"<<reg.name<<", "<<reg.offset_mul<<"U*BIT, "<<regMask<<", "<<reg.name<<'_'<<custVal.name<<"(VAL))\n"; 
            out_file_stream << std::endl;

            for (auto& bit : reg.bits)
            {
                std::string bitMask;
                if (bit.length == 1)
                {
                    bitMask = "0b1U";
                    out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<"() SET_BIT(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<")\n";
                    out_file_stream << "#define CLEAR_"<<reg.name<<'_'<<bit.name<<"() CLEAR_BIT(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<")\n";
                    out_file_stream << "#define RESET_"<<reg.name<<'_'<<bit.name<<"(VAL) RESET_BITS(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<", 0b1U, VAL)\n";
                    out_file_stream << "#define READ_"<<reg.name<<'_'<<bit.name<<"() READ_BIT(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<")\n";
                }
                else
                {
                    bitMask = "0b";
                    for (int k = 0; k < bit.length; k ++)
                        bitMask.push_back('1');
                    bitMask.push_back('U');

                    out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<"(VAL) SET_BITS(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<", VAL)\n";
                    out_file_stream << "#define RESET_"<<reg.name<<'_'<<bit.name<<"(VAL) RESET_BITS(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<", VAL)\n";
                    out_file_stream << "#define READ_"<<reg.name<<'_'<<bit.name<<"() READ_BITS(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<")\n";
                }
                out_file_stream << std::endl;

                for (auto& custVal : bit.custom_val) 
                    if (custVal.val.find("VAL") == std::string::npos)
                        out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<"() RESET_BITS(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<", "<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<")\n"; 
                    else
                        out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<"(VAL) RESET_BITS(REG_"<<reg.name<<", "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<", "<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<"(VAL))\n"; 
                out_file_stream << std::endl;
            }

            out_file_stream << std::endl;
        }
        else
        {
            std::string regMask;
            if (reg.offset_mul == 1)
            {
                regMask = "0b1U";
                out_file_stream << "#define SET_"<<reg.name<<"_BIT(X, BIT) SET_BIT(REG_"<<reg.name<<"(X), BIT)\n";
                out_file_stream << "#define CLEAR_"<<reg.name<<"_BIT(X, BIT) CLEAR_BIT(REG_"<<reg.name<<"(X), BIT)\n";
                out_file_stream << "#define RESET_"<<reg.name<<"_BIT(X, BIT, VAL) RESET_BIT(REG_"<<reg.name<<"(X), BIT, VAL)\n";
                out_file_stream << "#define READ_"<<reg.name<<"_BIT(X, BIT) READ_BIT(REG_"<<reg.name<<"(X), BIT)\n";
            }
            else
            {
                regMask = "0b";
                for (int k = 0; k < reg.offset_mul; k ++)
                    regMask.push_back('1');
                regMask.push_back('U');

                out_file_stream << "#define SET_"<<reg.name<<"_BITS(X, BIT, VAL) SET_BITS(REG_"<<reg.name<<"(X), "<<reg.offset_mul<<"U*BIT, VAL)\n";
                out_file_stream << "#define RESET_"<<reg.name<<"_BITS(X, BIT, VAL) RESET_BITS(REG_"<<reg.name<<"(X), "<<reg.offset_mul<<"U*BIT, "<<regMask<<", VAL)\n";
                out_file_stream << "#define READ_"<<reg.name<<"_BITS(X, BIT) READ_BITS(REG_"<<reg.name<<"(X), "<<reg.offset_mul<<"U*BIT, "<<regMask<<")\n";
            }

            for (auto& custVal : reg.custom_val) 
                if (custVal.val.find("VAL") == std::string::npos)
                    out_file_stream << "#define SET_"<<reg.name<<'_'<<custVal.name<<"(X, BIT) RESET_BITS(REG_"<<reg.name<<"(X), "<<reg.offset_mul<<"U*BIT, "<<regMask<<", "<<reg.name<<'_'<<custVal.name<<")\n"; 
                else
                    out_file_stream << "#define SET_"<<reg.name<<'_'<<custVal.name<<"(X, BIT, VAL) RESET_BITS(REG_"<<reg.name<<"(X), "<<reg.offset_mul<<"U*BIT, "<<regMask<<", "<<reg.name<<'_'<<custVal.name<<"(VAL))\n"; 
            out_file_stream << std::endl;

            for (auto& bit : reg.bits)
            {
                std::string bitMask;
                if (bit.length == 1)
                {
                    bitMask = "0b1U";
                    out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<"(X) SET_BIT(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<")\n";
                    out_file_stream << "#define CLEAR_"<<reg.name<<'_'<<bit.name<<"(X) CLEAR_BIT(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<")\n";
                    out_file_stream << "#define RESET_"<<reg.name<<'_'<<bit.name<<"(X, VAL) RESET_BITS(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<", 0b1U, VAL)\n";
                    out_file_stream << "#define READ_"<<reg.name<<'_'<<bit.name<<"(X) READ_BIT(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<")\n";
                }
                else
                {
                    bitMask = "0b";
                    for (int k = 0; k < bit.length; k ++)
                        bitMask.push_back('1');
                    bitMask.push_back('U');

                    out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<"(X, VAL) SET_BITS(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<", VAL)\n";
                    out_file_stream << "#define RESET_"<<reg.name<<'_'<<bit.name<<"(X, VAL) RESET_BITS(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<", VAL)\n";
                    out_file_stream << "#define READ_"<<reg.name<<'_'<<bit.name<<"(X) READ_BITS(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<")\n";
                }
                out_file_stream << std::endl;

                for (auto& custVal : bit.custom_val) 
                    if (custVal.val.find("VAL") == std::string::npos)
                        out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<"(X) RESET_BITS(REG_"<<reg.name<<"(X), "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<", "<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<")\n"; 
                    else
                        out_file_stream << "#define SET_"<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<"(X) RESET_BITS(REG_"<<reg.name<<"(X, VAL), "<<reg.name<<'_'<<bit.name<<", "<<bitMask<<", "<<reg.name<<'_'<<bit.name<<'_'<<custVal.name<<"(VAL))\n"; 
                out_file_stream << std::endl;
            }

            out_file_stream << std::endl;
        }
    }


    out_file_stream.close();

    return 0;

}
