#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <math.h>
#include <bitset>
#include <algorithm>
#include <math.h>
#include <iomanip>

struct SIM_INPUT
{
    unsigned int counter_bit = 0; 
    unsigned int init_counter = 0;
    unsigned int taken_max = 0;
    unsigned int not_max = 0;
    unsigned int miss_prdct = 0;
    unsigned int bimodal_idx_val = 0;
    unsigned int gshare_idx_val = 0;
    unsigned int chooser_val = 0;
    unsigned int BHR = 0;
    unsigned int BHR_bits = 0;
    std::vector<unsigned int> gshare_prdct_table;
    std::vector<unsigned int> bimodal_prdct_table;
    std::vector<unsigned int> chooser_table;
    std::vector<unsigned int> gshare_idx_table;
    std::vector<unsigned int> bimodal_idx_table;
    std::vector<unsigned int> chooser_idx_table;
    
} my_input;

std::string predictor_type = " ";
std::string trace_file = " ";
std::vector<std::string> file_content;
std::vector<unsigned int> tag_content;
std::string predict = "";
double miss_rate = 0.0;
void sim_smith(std::string act_predct, struct SIM_INPUT *st);
void sim_bimodal(std::string act_predct, struct SIM_INPUT *st, int itr);
void sim_gshare(std::string act_predct, struct SIM_INPUT *st, int itr);
void sim_hybrid(std::string act_predct, struct SIM_INPUT *st, int itr);
int calcIdx(int val, unsigned int val2);
void modify_bhr(std::string actual_prd);

int main(int argc, char *argv[])
{

    char *pCh;
    predictor_type = argv[1];
    trace_file = argv[argc - 1];
    
    std::string token = "";
    std::vector<std::string> input;
    unsigned int hexAd = 0;
    std::fstream myfile(trace_file);
    std::string line = "";
    unsigned int idx_bits = 0;
    unsigned int fileLen = 1;

    // start get trace file data. //
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            if (line.empty())
            {
                continue;
            }
            file_content.push_back(line);
        }
        myfile.close();
    }
    else
    {
        std::cout << "**" << trace_file << " doesn't exist.**" << std::endl;
        std::cout << "**Please make sure that the trace files are in the same directory as sim executable.**" << std::endl;
    }
    // end get trace file data //

    // smith // 
    if (predictor_type == "smith")
    {

        SIM_INPUT *input = &my_input;

        input->counter_bit = strtoul(argv[2], &pCh, 10);
        input->init_counter = pow(2, (input->counter_bit - 1));
        input->taken_max = (pow(2, (input->counter_bit))) - 1;
        input->not_max = input->init_counter - 1;
    }

    // bimodal //
    // argv[1] = bimodal, argv[2] = idx_bits, argv[3] = trace file name
    else if (predictor_type == "bimodal")
    {
        SIM_INPUT *bimodal_input = &my_input;

        bimodal_input->bimodal_idx_val = strtoul(argv[2], &pCh, 10);
        bimodal_input->bimodal_prdct_table.resize((pow(2, (bimodal_input->bimodal_idx_val))), 4);

        // create index table
        for (int i = 0; i < file_content.size(); i++)
        {
            std::istringstream stm(file_content[i]);
            input.clear();
            while (stm >> token)
            {
                input.push_back(token);
            }
            hexAd = strtoul(input[0].c_str(), 0, 16);
            idx_bits = calcIdx(hexAd, bimodal_input->bimodal_idx_val);
            bimodal_input->bimodal_idx_table.push_back(idx_bits);
        }
    }
    // gshare //
    // argv[0] = ./sim, argv[1] = gshare, argv[2] = 11, argv[3] = 5, argv[4] = jpeg_trace.txt
    if (predictor_type == "gshare")
    {

        SIM_INPUT *gshare_input = &my_input;

        gshare_input->gshare_idx_val = strtoul(argv[2], &pCh, 10);
        gshare_input->gshare_prdct_table.resize((pow(2, (gshare_input->gshare_idx_val))), 4);
        gshare_input->BHR_bits = strtoul(argv[3], &pCh, 10);

        // create index table
        for (int i = 0; i < file_content.size(); i++)
        {
            std::istringstream stm(file_content[i]);
            input.clear();
            while (stm >> token)
            {
                input.push_back(token);
            }
            hexAd = strtoul(input[0].c_str(), 0, 16);
            idx_bits = calcIdx(hexAd, gshare_input->gshare_idx_val);
            gshare_input->gshare_idx_table.push_back(idx_bits);
        }
    }

    // hybrid //
    // argv[0] = ./sim, argv[1] = hybrid, argv[2] = 8, argv[3] = 14, argv[4] = 10, argv[5] = 5, argv[6] = gcc_trace.txt
    else if (predictor_type == "hybrid")
    {
        SIM_INPUT *hybrid_input = &my_input;
        // populate chooser table
        hybrid_input->chooser_val = strtoul(argv[2], &pCh, 10);
        hybrid_input->chooser_table.resize((pow(2, (hybrid_input->chooser_val))), 1);

        //populate gshare predict table and bhr value
        hybrid_input->gshare_idx_val = strtoul(argv[3], &pCh, 10);
        hybrid_input->gshare_prdct_table.resize((pow(2, (hybrid_input->gshare_idx_val))), 4);
        hybrid_input->BHR_bits = strtoul(argv[4], &pCh, 10);

        // populate bimodal predict table
        hybrid_input->bimodal_idx_val = strtoul(argv[5], &pCh, 10);
        hybrid_input->bimodal_prdct_table.resize((pow(2, (hybrid_input->bimodal_idx_val))), 4);

        for (int i = 0; i < file_content.size(); i++)
        {
            std::istringstream stm(file_content[i]);
            input.clear();
            while (stm >> token)
            {
                input.push_back(token);
            }
            hexAd = strtoul(input[0].c_str(), 0, 16);

            //bimodal
            idx_bits = calcIdx(hexAd, hybrid_input->gshare_idx_val);
            hybrid_input->gshare_idx_table.push_back(idx_bits);

            // gshare
            idx_bits = calcIdx(hexAd, hybrid_input->bimodal_idx_val);
            hybrid_input->bimodal_idx_table.push_back(idx_bits);

            // chooser table
            idx_bits = calcIdx(hexAd, hybrid_input->chooser_val);
            hybrid_input->chooser_idx_table.push_back(idx_bits);
        }
    }

    // start simulation //
    for (int i = 0; i < file_content.size(); i++)
    {
        std::istringstream stm(file_content[i]);
        input.clear();
        while (stm >> token)
        {
            input.push_back(token);
        }
        if (predictor_type == "smith")
        {
            sim_smith(input[1], &my_input);
        }
        else if (predictor_type == "bimodal")
        {
            sim_bimodal(input[1], &my_input, i);
        }
        else if (predictor_type == "gshare")
        {
            sim_gshare(input[1], &my_input, i);
        }
        else if (predictor_type == "hybrid")
        {
            sim_hybrid(input[1], &my_input, i);
        }
    }
    // end simulation //

    // calculate misprediction rate
    miss_rate = (my_input.miss_prdct / double(file_content.size())) * 100;
    miss_rate = std::round(miss_rate * 100) / 100;

    // start print //
    if (predictor_type == "smith" || predictor_type == "bimodal")
    {
        std::cout << "COMMAND" << std::endl;
        std::cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << trace_file << std::endl;
        std::cout << "OUTPUT" << std::endl;
        std::cout << "number of predictions:        " << file_content.size() << std::endl;
        std::cout << "number of mispredictions:     " << my_input.miss_prdct << std::endl;
        std::cout << std::fixed << std::setprecision(2) << "misprediction rate:           "
                  << miss_rate << "%" << std::endl;

        if (predictor_type == "smith")
        {
            std::cout << "FINAL COUNTER CONTENT:        " << my_input.init_counter << std::endl;
        }
        else if (predictor_type == "bimodal")
        {
            std::cout << "FINAL BIMODAL CONTENTS" << std::endl;
            for (int i = 0; i < my_input.bimodal_prdct_table.size(); i++)
            {
                std::cout << i << "\t" << my_input.bimodal_prdct_table[i] << std::endl;
            }
        }
    }

    else if (predictor_type == "gshare")
    {
        std::cout << "COMMAND" << std::endl;
        std::cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << trace_file << std::endl;
        std::cout << "OUTPUT" << std::endl;
        std::cout << "number of predictions:        " << file_content.size() << std::endl;
        std::cout << "number of mispredictions:     " << my_input.miss_prdct << std::endl;
        std::cout << std::fixed << std::setprecision(2) << "misprediction rate:           " << miss_rate << "%" << std::endl;
        std::cout << "FINAL GSHARE CONTENTS" << std::endl;
        for (int i = 0; i < my_input.gshare_prdct_table.size(); i++)
        {
            std::cout << i << "\t" << my_input.gshare_prdct_table[i] << std::endl;
        }
    }
    else if (predictor_type == "hybrid")
    {
        std::cout << "COMMAND" << std::endl;
        std::cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " "
                  << argv[5] << " " << trace_file << std::endl;
        std::cout << "OUTPUT" << std::endl;
        std::cout << "number of predictions:        " << file_content.size() << std::endl;
        std::cout << "number of mispredictions:     " << my_input.miss_prdct << std::endl;
        std::cout << std::fixed << std::setprecision(2) << "misprediction rate:           " << miss_rate << "%" << std::endl;
        std::cout << "FINAL CHOOSER CONTENTS" << std::endl;
        for (int i = 0; i < my_input.chooser_table.size(); i++)
        {
            std::cout << i << "\t" << my_input.chooser_table[i] << std::endl;
        }
        std::cout << "FINAL GSHARE CONTENTS" << std::endl;
        for (int i = 0; i < my_input.gshare_prdct_table.size(); i++)
        {
            std::cout << i << "\t" << my_input.gshare_prdct_table[i] << std::endl;
        }
        std::cout << "FINAL BIMODAL CONTENTS" << std::endl;
        for (int i = 0; i < my_input.bimodal_prdct_table.size(); i++)
        {
            std::cout << i << "\t" << my_input.bimodal_prdct_table[i] << std::endl;
        }
    }

    // end print //
}

void modify_bhr(std::string actual_prd)
{
    const int bit = my_input.BHR_bits;

    if (actual_prd == "t")
    {
        my_input.BHR = my_input.BHR >> 1;
        std::bitset<32> adr(my_input.BHR);
        std::string mystring = adr.to_string<char, std::string::traits_type, std::string::allocator_type>();

        mystring[mystring.size() - my_input.BHR_bits] = '1';
        std::bitset<32> IDX(mystring, 0, 32);
        my_input.BHR = IDX.to_ulong();
    }
    else if (actual_prd == "n")
    {
        my_input.BHR = my_input.BHR >> 1;

        std::bitset<32> adr(my_input.BHR);
        std::string mystring = adr.to_string<char, std::string::traits_type, std::string::allocator_type>();

        mystring[mystring.size() - my_input.BHR_bits] = '0';
        std::bitset<32> IDX(mystring, 0, 32);
        my_input.BHR = IDX.to_ulong();
    }
}

int calcIdx(int val, unsigned int val2)
{
    unsigned int num = 0;
    std::bitset<32> adr(val);

    std::string mystring = adr.to_string<char, std::string::traits_type, std::string::allocator_type>();

    std::bitset<32> IDX(mystring, 30 - val2, val2);

    num = IDX.to_ulong();

    return num;
}

void sim_smith(std::string act_predct, struct SIM_INPUT *st)
{

    if (st->init_counter > st->not_max)
    {
        predict = "t";
        if (predict == act_predct)
        {
            if (st->init_counter < st->taken_max)
            {
                st->init_counter++;
            }
        }
        else
        {
            st->miss_prdct++;
            if (st->init_counter > 0)
            {
                st->init_counter--;
            }
        }
    }
    else if (st->init_counter <= st->not_max)
    {
        predict = "n";
        if (predict == act_predct)
        {
            if (st->init_counter > 0)
            {
                st->init_counter--;
            }
        }
        else
        {
            st->miss_prdct++;
            if (st->init_counter < st->taken_max)
            {
                st->init_counter++;
            }
        }
    }
}

void sim_bimodal(std::string act_predct, struct SIM_INPUT *st, int itr)
{
    if (st->bimodal_prdct_table[st->bimodal_idx_table[itr]] >= 4)
    {
        predict = "t";
        if (act_predct == predict)
        {
            if (st->bimodal_prdct_table[st->bimodal_idx_table[itr]] < 7)
            {
                st->bimodal_prdct_table[st->bimodal_idx_table[itr]]++;
            }
        }
        else
        {
            st->miss_prdct++;
            if (st->bimodal_prdct_table[st->bimodal_idx_table[itr]] > 0)
            {
                st->bimodal_prdct_table[st->bimodal_idx_table[itr]]--;
            }
        }
    }

    else if (st->bimodal_prdct_table[st->bimodal_idx_table[itr]] < 4)
    {
        predict = "n";
        if (act_predct == predict)
        {
            if (st->bimodal_prdct_table[st->bimodal_idx_table[itr]] > 0)
            {
                st->bimodal_prdct_table[st->bimodal_idx_table[itr]]--;
            }
        }
        else
        {
            st->miss_prdct++;
            if (st->bimodal_prdct_table[st->bimodal_idx_table[itr]] < 7)
            {
                st->bimodal_prdct_table[st->bimodal_idx_table[itr]]++;
            }
        }
    }
}

void sim_gshare(std::string act_predct, struct SIM_INPUT *st, int itr)
{
    unsigned int adr = st->gshare_idx_table[itr] ^ st->BHR;

    if (st->gshare_prdct_table[adr] >= 4)
    {
        predict = "t";
        if (act_predct == predict)
        {
            if (st->gshare_prdct_table[adr] < 7)
            {
                st->gshare_prdct_table[adr]++;
            }
            modify_bhr(act_predct);
        }
        else
        {
            st->miss_prdct++;
            if (st->gshare_prdct_table[adr] > 0)
            {
                st->gshare_prdct_table[adr]--;
            }
            modify_bhr(act_predct);
        }
    }

    else if (st->gshare_prdct_table[adr] < 4)
    {
        predict = "n";
        if (act_predct == predict)
        {
            if (st->gshare_prdct_table[adr] > 0)
            {
                st->gshare_prdct_table[adr]--;
            }
            modify_bhr(act_predct);
        }
        else
        {
            st->miss_prdct++;
            if (st->gshare_prdct_table[adr] < 7)
            {
                st->gshare_prdct_table[adr]++;
            }
            modify_bhr(act_predct);
        }
    }
}

void mod_chooser(std::string act_prdct, int itr)
{
    std::string prd_gshare, prd_bimodal;
    unsigned int adr = my_input.gshare_idx_table[itr] ^ my_input.BHR;

    prd_gshare = (my_input.gshare_prdct_table[adr] >= 4) ? "t" : "n";
    prd_bimodal = (my_input.bimodal_prdct_table[my_input.bimodal_idx_table[itr]] >= 4) ? "t" : "n";

    if (prd_gshare == act_prdct & prd_bimodal != act_prdct)
    {
        if (my_input.chooser_table[my_input.chooser_idx_table[itr]] < 3)
        {
            my_input.chooser_table[my_input.chooser_idx_table[itr]]++;
        }
    }
    else if (prd_gshare != act_prdct & prd_bimodal == act_prdct)
    {
        if (my_input.chooser_table[my_input.chooser_idx_table[itr]] > 0)
        {
            my_input.chooser_table[my_input.chooser_idx_table[itr]]--;
        }
    }
}

void sim_hybrid(std::string act_predct, struct SIM_INPUT *st, int itr)
{

    if (st->chooser_table[st->chooser_idx_table[itr]] >= 2)
    {
        mod_chooser(act_predct, itr);
        sim_gshare(act_predct, st, itr);
    }
    else
    {
        mod_chooser(act_predct, itr);
        modify_bhr(act_predct);
        sim_bimodal(act_predct, st, itr);
    }
}
