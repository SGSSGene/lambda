#include <fstream>
#include <string>

#include "cli_test.hpp"

struct search_test : public cli_test
{
    std::string md5_cmd;

    void determine_md5_cmd()
    {
        if (std::system("echo foo | md5sum > /dev/null 2>&1") == 0)
            md5_cmd = "md5sum";
        else if (std::system("echo foo | md5 > /dev/null 2>&1") == 0)
            md5_cmd = "md5";

        ASSERT_FALSE(md5_cmd.empty());
    }

    void run_search_test(std::string const & index_command,
                         std::string const & db_file,
                         std::string const & index_file,
                         std::string const & index_type,
                         std::string const & reduction,
                         std::string const & search_command,
                         std::string const & query_file,
                         std::string const & output_file,
                         std::string const & output_type,
                         std::string const & control_file)
    {
        cli_test_result result_index = execute_app("lambda3", index_command,
                                                   "-d", data(db_file),
                                                   "-i", index_file,
                                                   "--db-index-type", index_type,
                                                   "-r", reduction);

        cli_test_result result_search = execute_app("lambda3", search_command,
                                                    "-i", index_file,
                                                    "-q", data(query_file),
                                                    "-t", "2",
                                                    "--version-to-outputfile", "0",
                                                    "-o", output_file);

        ASSERT_EQ(result_search.exit_code, 0);

        int ret = 0;

        if (output_type == "bam")
        {
            if (md5_cmd.empty())
                determine_md5_cmd();

            ret = std::system((md5_cmd + " " + output_file + " > md5sum_test.txt").c_str());
            ASSERT_TRUE(ret == 0);
            ret = std::system((md5_cmd + " " + (std::string) data(control_file) + " > md5sum_control.txt").c_str());
            ASSERT_TRUE(ret == 0);

            std::ifstream test_output ("md5sum_test.txt");
            std::ifstream control_output ("md5sum_control.txt");

            std::string test_line;
            std::string control_line;

            getline(test_output, test_line);
            getline(control_output, control_line);

            std::string test_sum = test_line.substr(0, test_line.find(' '));
            std::string control_sum = control_line.substr(0, control_line.find(' '));

            ASSERT_EQ(test_sum, control_sum);
        }
        else
        {
            std::ifstream test_output;

            if (output_type == "m9_gz")
            {
                ret = system(("gunzip " + output_file).c_str());
                ASSERT_TRUE(ret == 0);
                test_output.open(output_file.substr(0, output_file.length() - 3));
            }
            else if (output_type == "sam_bz2")
            {
                ret = system(("bzip2 -d " + output_file).c_str());
                ASSERT_TRUE(ret == 0);
                test_output.open(output_file.substr(0, output_file.length() - 4));
            }
            else
            {
                test_output.open(output_file);
            }

            std::ifstream control_output (data(control_file));

            std::string test_line;
            std::string control_line;
            while (!test_output.eof() && !control_output.eof())
            {
                getline(test_output, test_line);
                getline(control_output, control_line);
                ASSERT_EQ(test_line, control_line);
            }
            EXPECT_TRUE(test_output.eof());
            EXPECT_TRUE(control_output.eof());
        }
    }
};

TEST_F(search_test, searchn_no_options)
{
    cli_test_result result = execute_app("lambda3", "searchn");
    std::string expected
    {
        "lambda3-searchn - the Local Aligner for Massive Biological DatA\n"
        "===============================================================\n"
        "    [OPTIONS] -q QUERY.fasta -i INDEX.lambda [-o output.m8]\n"
        "    Try -h or --help for more information.\n"
    };
    ASSERT_EQ(result.exit_code, 0);
    ASSERT_EQ(result.out, expected);
    ASSERT_EQ(result.err, std::string{});
}

TEST_F(search_test, searchp_no_options)
{
    cli_test_result result = execute_app("lambda3", "searchp");
    std::string expected
    {
        "lambda3-searchp - the Local Aligner for Massive Biological DatA\n"
        "===============================================================\n"
        "    [OPTIONS] -q QUERY.fasta -i INDEX.lambda [-o output.m8]\n"
        "    Try -h or --help for more information.\n"
    };
    ASSERT_EQ(result.exit_code, 0);
    ASSERT_EQ(result.out, expected);
    ASSERT_EQ(result.err, std::string{});
}

// BLASTN

TEST_F(search_test, blastn_fm_m0)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_fm_test.m0", "m0", "output_blastn_fm.m0");
}

TEST_F(search_test, blastn_fm_m8)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_fm_test.m8", "m8", "output_blastn_fm.m8");
}

TEST_F(search_test, blastn_fm_m9)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_fm_test.m9", "m9", "output_blastn_fm.m9");
}

TEST_F(search_test, blastn_fm_m9_gz)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_fm_test.m9.gz", "m9_gz", "output_blastn_fm.m9");
}

TEST_F(search_test, blastn_fm_sam)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_fm_test.sam", "sam", "output_blastn_fm.sam");
}

TEST_F(search_test, blastn_fm_sam_bz2)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_fm_test.sam.bz2", "sam_bz2", "output_blastn_fm.sam");
}

TEST_F(search_test, blastn_fm_bam)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_fm_test.bam", "bam", "output_blastn_fm.bam");
}

// BLASTN bisulfite mode

TEST_F(search_test, blastn_bs_fm_m0)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_fm.fasta.gz.lba", "fm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_fm_test.m0", "m0", "output_blastn_bs_fm.m0");
}

TEST_F(search_test, blastn_bs_fm_m8)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_fm.fasta.gz.lba", "fm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_fm_test.m8", "m8", "output_blastn_bs_fm.m8");
}

TEST_F(search_test, blastn_bs_fm_m9)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_fm.fasta.gz.lba", "fm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_fm_test.m9", "m9", "output_blastn_bs_fm.m9");
}

TEST_F(search_test, blastn_bs_fm_m9_gz)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_fm.fasta.gz.lba", "fm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_fm_test.m9.gz", "m9_gz", "output_blastn_bs_fm.m9");
}

TEST_F(search_test, blastn_bs_fm_sam)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_fm.fasta.gz.lba", "fm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_fm_test.sam", "sam", "output_blastn_bs_fm.sam");
}

TEST_F(search_test, blastn_bs_fm_sam_bz2)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_fm.fasta.gz.lba", "fm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_fm_test.sam.bz2", "sam_bz2", "output_blastn_bs_fm.sam");
}

TEST_F(search_test, blastn_bs_fm_bam)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_fm.fasta.gz.lba", "fm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_fm_test.bam", "bam", "output_blastn_bs_fm.bam");
}

// BLASTP

TEST_F(search_test, blastp_fm_m0)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_fm_test.m0", "m0", "output_blastp_fm.m0");
}

TEST_F(search_test, blastp_fm_m8)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_fm_test.m8", "m8", "output_blastp_fm.m8");
}

TEST_F(search_test, blastp_fm_m9)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_fm_test.m9", "m9", "output_blastp_fm.m9");
}

TEST_F(search_test, blastp_fm_m9_gz)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_fm_test.m9.gz", "m9_gz", "output_blastp_fm.m9");
}

TEST_F(search_test, blastp_fm_sam)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_fm_test.sam", "sam", "output_blastp_fm.sam");
}

TEST_F(search_test, blastp_fm_sam_bz2)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_fm_test.sam.bz2", "sam_bz2", "output_blastp_fm.sam");
}

TEST_F(search_test, blastp_fm_bam)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_fm_test.bam", "bam", "output_blastp_fm.bam");
}

// BLASTX

TEST_F(search_test, blastx_fm_m0)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_fm_test.m0", "m0", "output_blastx_fm.m0");
}

TEST_F(search_test, blastx_fm_m8)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_fm_test.m8", "m8", "output_blastx_fm.m8");
}

TEST_F(search_test, blastx_fm_m9)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_fm_test.m9", "m9", "output_blastx_fm.m9");
}

TEST_F(search_test, blastx_fm_m9_gz)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_fm_test.m9.gz", "m9_gz", "output_blastx_fm.m9");
}

TEST_F(search_test, blastx_fm_sam)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_fm_test.sam", "sam", "output_blastx_fm.sam");
}

TEST_F(search_test, blastx_fm_sam_bz2)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_fm_test.sam.bz2", "sam_bz2", "output_blastx_fm.sam");
}

TEST_F(search_test, blastx_fm_bam)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_fm_test.bam", "bam", "output_blastx_fm.bam");
}

// TBLASTN

TEST_F(search_test, tblastn_fm_m0)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_fm_test.m0", "m0", "output_tblastn_fm.m0");
}

TEST_F(search_test, tblastn_fm_m8)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_fm_test.m8", "m8", "output_tblastn_fm.m8");
}

TEST_F(search_test, tblastn_fm_m9)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_fm_test.m9", "m9", "output_tblastn_fm.m9");
}

TEST_F(search_test, tblastn_fm_m9_gz)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_fm_test.m9.gz", "m9_gz", "output_tblastn_fm.m9");
}

TEST_F(search_test, tblastn_fm_sam)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_fm_test.sam", "sam", "output_tblastn_fm.sam");
}

TEST_F(search_test, tblastn_fm_sam_bz2)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_fm_test.sam.bz2", "sam_bz2", "output_tblastn_fm.sam");
}

TEST_F(search_test, tblastn_fm_bam)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_fm_test.bam", "bam", "output_tblastn_fm.bam");
}

// TBLASTX

TEST_F(search_test, tblastx_fm_m0)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_fm_test.m0", "m0", "output_tblastx_fm.m0");
}

TEST_F(search_test, tblastx_fm_m8)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_fm_test.m8", "m8", "output_tblastx_fm.m8");
}

TEST_F(search_test, tblastx_fm_m9)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_fm_test.m9", "m9", "output_tblastx_fm.m9");
}

TEST_F(search_test, tblastx_fm_m9_gz)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_fm_test.m9.gz", "m9_gz", "output_tblastx_fm.m9");
}

TEST_F(search_test, tblastx_fm_sam)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_fm_test.sam", "sam", "output_tblastx_fm.sam");
}

TEST_F(search_test, tblastx_fm_sam_bz2)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_fm_test.sam.bz2", "sam_bz2", "output_tblastx_fm.sam");
}

TEST_F(search_test, tblastx_fm_bam)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_nucl_fm.fasta.gz.lba", "fm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_fm_test.bam", "bam", "output_tblastx_fm.bam");
}

// Search with bi-directional index

TEST_F(search_test, blastn_bifm_m8)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_bifm.fasta.gz.lba", "bifm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_bifm_test.m8", "m8", "output_blastn_bifm.m8");
}

TEST_F(search_test, blastn_bifm_sam)
{
    run_search_test("mkindexn", "db_nucl.fasta.gz", "db_nucl_bifm.fasta.gz.lba", "bifm", "dna4", "searchn",
                    "queries_nucl.fasta.gz", "output_blastn_bifm_test.sam", "sam", "output_blastn_bifm.sam");
}

TEST_F(search_test, blastn_bs_bifm_m8)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_bifm.fasta.gz.lba", "bifm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_bifm_test.m8", "m8", "output_blastn_bs_bifm.m8");
}

TEST_F(search_test, blastn_bs_bifm_sam)
{
    run_search_test("mkindexn", "db_nucl_bs.fasta.gz", "db_nucl_bs_bifm.fasta.gz.lba", "bifm", "dna3bs", "searchn",
                    "queries_nucl_bs.fasta.gz", "output_blastn_bs_bifm_test.sam", "sam", "output_blastn_bs_bifm.sam");
}

TEST_F(search_test, blastp_bifm_m8)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_bifm_test.m8", "m8", "output_blastp_bifm.m8");
}

TEST_F(search_test, blastp_bifm_sam)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_blastp_bifm_test.sam", "sam", "output_blastp_bifm.sam");
}

TEST_F(search_test, blastx_bifm_m8)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_bifm_test.m8", "m8", "output_blastx_bifm.m8");
}

TEST_F(search_test, blastx_bifm_sam)
{
    run_search_test("mkindexp", "db_prot.fasta.gz", "db_prot_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_blastx_bifm_test.sam", "sam", "output_blastx_bifm.sam");
}

TEST_F(search_test, tblastn_bifm_m8)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_bifm_test.m8", "m8", "output_tblastn_bifm.m8");
}

TEST_F(search_test, tblastn_bifm_sam)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_prot.fasta.gz", "output_tblastn_bifm_test.sam", "sam", "output_tblastn_bifm.sam");
}

TEST_F(search_test, tblastx_bifm_m8)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_bifm_test.m8", "m8", "output_tblastx_bifm.m8");
}

TEST_F(search_test, tblastx_bifm_sam)
{
    run_search_test("mkindexp", "db_nucl.fasta.gz", "db_trans_bifm.fasta.gz.lba", "bifm", "murphy10", "searchp",
                    "queries_nucl.fasta.gz", "output_tblastx_bifm_test.sam", "sam", "output_tblastx_bifm.sam");
}
