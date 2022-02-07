// ==========================================================================
//                                  lambda
// ==========================================================================
// Copyright (c) 2013-2020, Hannes Hauswedell <h2 @ fsfe.org>
// Copyright (c) 2016-2020, Knut Reinert and Freie Universität Berlin
// All rights reserved.
//
// This file is part of Lambda.
//
// Lambda is Free Software: you can redistribute it and/or modify it
// under the terms found in the LICENSE[.md|.rst] file distributed
// together with this file.
//
// Lambda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// ==========================================================================
// mkindex.cpp: Main File for building the index
// ==========================================================================

#include <initializer_list>
#include <iostream>

#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

#define LAMBDA_INDEXER 1 // some things are different for the indexer binary

#include "shared_options.hpp"
#include "shared_definitions.hpp"
#include "shared_misc.hpp"

#include "mkindex_misc.hpp"
#include "mkindex_options.hpp"
// #include "mkindex_saca.hpp"
#include "mkindex_algo.hpp"
#include "view_reduce_to_bisulfite.hpp"

// ==========================================================================
// Forwards
// ==========================================================================

void argConv0(LambdaIndexerOptions & options);

template <DbIndexType c_indexType>
void argConv1(LambdaIndexerOptions & options);

template <DbIndexType   c_indexType,
          AlphabetEnum  c_origAlph>
void argConv2(LambdaIndexerOptions const & options);

template <DbIndexType   c_indexType,
          AlphabetEnum  c_origAlph,
          AlphabetEnum  c_transAlph>
void argConv3a(LambdaIndexerOptions const & options);

template <DbIndexType   c_indexType,
          AlphabetEnum  c_origAlph,
          AlphabetEnum  c_transAlph>
void argConv3b(LambdaIndexerOptions const & options);


template <DbIndexType           c_indexType,
          AlphabetEnum          c_origAlph,
          AlphabetEnum          c_transAlph,
          AlphabetEnum          c_redAlph>
void realMain(LambdaIndexerOptions     const & options);

// --------------------------------------------------------------------------
// Function main()
// --------------------------------------------------------------------------

// Program entry point.

int mkindexMain(int const argc, char const ** argv)
{
    LambdaIndexerOptions options;
    parseCommandLine(options, argc, argv);

#ifdef NDEBUG
    try
    {
        argConv0(options);
    } catch (std::bad_alloc const & e)
    {
        std::cerr << "ERROR: Lambda ran out of memory :(\n"
                     "       You need to split your file into smaller segments.\n";
        return -1;
    } catch (std::exception const & e)
    {
        std::cerr << "\n\nERROR: The following unspecified exception was thrown:\n"
                  <<     "       \"" << e.what() << "\"\n"
                  <<     "       If the problem persists, report an issue at https://github.com/seqan/lambda/issues "
                  << "and include this output, as well as the output of `lambda2 --version`, thanks!\n";
        return -1;
    }
#else
    // In debug mode we don't catch the exceptions so that we get a backtrace from SeqAn's handler
    argConv0(options);
#endif
    return 0;
}

void argConv0(LambdaIndexerOptions & options)
{
    switch (options.indexFileOptions.indexType)
    {
        case DbIndexType::FM_INDEX:         return argConv1<DbIndexType::FM_INDEX>(options);
        case DbIndexType::BI_FM_INDEX:      return argConv1<DbIndexType::BI_FM_INDEX>(options);
        case DbIndexType::BI_FM_INDEX_SGG:  return argConv1<DbIndexType::BI_FM_INDEX_SGG>(options);
        default:                            throw 42;
    }
}

template <DbIndexType c_indexType>
void argConv1(LambdaIndexerOptions & options)
{
    if (options.indexFileOptions.origAlph == AlphabetEnum::UNDEFINED)
    {
        myPrint(options, 1, "Detecting database alphabet... ");
        options.indexFileOptions.origAlph = detectSeqFileAlphabet(options.dbFile);
        myPrint(options, 1, _alphabetEnumToName(options.indexFileOptions.origAlph), " detected.\n");
        myPrint(options, 2, "\n");
    }

    switch (options.indexFileOptions.origAlph)
    {
        case AlphabetEnum::DNA5:            return argConv2<c_indexType, AlphabetEnum::DNA5>(options);
        case AlphabetEnum::AMINO_ACID:      return argConv3a<c_indexType, AlphabetEnum::AMINO_ACID, AlphabetEnum::AMINO_ACID>(options);
        default:                            throw 43;
    }
}

template <DbIndexType   c_indexType,
          AlphabetEnum  c_origAlph>
void argConv2(LambdaIndexerOptions const & options)
{
    switch (options.indexFileOptions.transAlph)
    {
        case AlphabetEnum::DNA5:            return argConv3b<c_indexType, c_origAlph, AlphabetEnum::DNA5>(options);
        case AlphabetEnum::AMINO_ACID:      return argConv3a<c_indexType, c_origAlph, AlphabetEnum::AMINO_ACID>(options);
        default:                            throw 44;
    }
}

template <DbIndexType   c_indexType,
          AlphabetEnum  c_origAlph,
          AlphabetEnum  c_transAlph>
void argConv3a(LambdaIndexerOptions const & options)
{
    switch (options.indexFileOptions.redAlph)
    {
        case AlphabetEnum::AMINO_ACID:      return realMain<c_indexType, c_origAlph, c_transAlph, AlphabetEnum::AMINO_ACID>(options);
        case AlphabetEnum::MURPHY10:        return realMain<c_indexType, c_origAlph, c_transAlph, AlphabetEnum::MURPHY10>(options);
        case AlphabetEnum::LI10:            return realMain<c_indexType, c_origAlph, c_transAlph, AlphabetEnum::LI10>(options);
        //TODO other reduced alphabets
        default:                            throw 45;
    }
}

template <DbIndexType   c_indexType,
          AlphabetEnum  c_origAlph,
          AlphabetEnum  c_transAlph>
void argConv3b(LambdaIndexerOptions const & options)
{
    switch (options.indexFileOptions.redAlph)
    {
        case AlphabetEnum::DNA5:       return realMain<c_indexType, c_origAlph, c_transAlph, AlphabetEnum::DNA5>(options);
        case AlphabetEnum::DNA4:       return realMain<c_indexType, c_origAlph, c_transAlph, AlphabetEnum::DNA4>(options);
        case AlphabetEnum::DNA3BS:     return realMain<c_indexType, c_origAlph, c_transAlph, AlphabetEnum::DNA3BS>(options);
        default:                       throw 364;
    }
}

template <DbIndexType   c_dbIndexType,
          AlphabetEnum  c_origAlph,
          AlphabetEnum  c_transAlph,
          AlphabetEnum  c_redAlph>
void realMain(LambdaIndexerOptions     const & options)
{
    index_file<c_dbIndexType, c_origAlph, c_redAlph> f;
    f.options = options.indexFileOptions;

    using TOrigSbjAlph = _alphabetEnumToType<c_origAlph>;
    using TTransSbjAlph = _alphabetEnumToType<c_transAlph>;
    using TRedSbjAlph = _alphabetEnumToType<c_redAlph>;

    {
        std::unordered_map<std::string, uint64_t> accToIdRank;

        // ids get saved to disk again immediately and are not kept in memory
        std::tie(f.ids, f.seqs, accToIdRank) = loadSubjSeqsAndIds<TOrigSbjAlph>(options);

        if (options.hasSTaxIds)
        {
            std::vector<bool> taxIdIsPresent;

            // read the mapping file and save relevant mappings to disk
            std::tie(f.sTaxIds, taxIdIsPresent) = mapTaxIDs(accToIdRank, std::ranges::size(f.seqs), options);

            // read the mapping file and save relevant mappings to disk
            std::tie(f.taxonParentIDs, f.taxonHeights, f.taxonNames) = parseAndStoreTaxTree(taxIdIsPresent, options);
        }
    }

    // see if final sequence set actually fits into index
//     checkIndexSize(translatedSeqs, options, seqan::BlastProgramSelector<p>());

    using TSbjSeqs = TCDStringSet<std::vector<TOrigSbjAlph>>;

    using TTransSbjSeqs = typename TTransSbjSeqsImpl<TSbjSeqs, TOrigSbjAlph, TTransSbjAlph, TRedSbjAlph>::type;
    using TRedSbjSeqs   = typename TRedSeqsImpl<TTransSbjSeqs, TTransSbjAlph, TRedSbjAlph>::type;

    TTransSbjSeqs       transSbjSeqs;
    TRedSbjSeqs         redSbjSeqs;

    if constexpr (c_redAlph == AlphabetEnum::DNA3BS)
        transSbjSeqs = f.seqs | views::duplicate;
    else if constexpr (c_origAlph == c_transAlph)
        transSbjSeqs = f.seqs | seqan3::views::type_reduce;     // no-op view
    else
        transSbjSeqs = f.seqs | seqan3::views::translate_join;

    if constexpr (c_transAlph == c_redAlph)
    {
        redSbjSeqs = transSbjSeqs | seqan3::views::type_reduce; // no-op view
    }
    else
    {
        if constexpr (c_transAlph == AlphabetEnum::AMINO_ACID)
            redSbjSeqs = transSbjSeqs | seqan3::views::deep{seqan3::views::convert<TRedSbjAlph>};
        else if constexpr (c_redAlph == AlphabetEnum::DNA3BS)
            redSbjSeqs = transSbjSeqs | views::dna_n_to_random
                                      | views::reduce_to_bisulfite;
        else
            redSbjSeqs = transSbjSeqs | views::dna_n_to_random;
    }

    f.index = generateIndex<c_dbIndexType>(redSbjSeqs, options);

    myPrint(options, 1, "Writing Index to disk...");
    double s = sysTime();
    if (options.indexFilePath.extension() == ".lba")
    {
        std::ofstream os{options.indexFilePath.c_str()};
        cereal::BinaryOutputArchive oarchive(os);
        oarchive(cereal::make_nvp("lambda index", f));
    } else if (options.indexFilePath.extension() == ".lta")
    {
        std::ofstream os{options.indexFilePath.c_str()};
        cereal::JSONOutputArchive oarchive(os);
        oarchive(cereal::make_nvp("lambda index", f));
    } else
    {
        throw 59;
    }

    double e = sysTime() - s;
    myPrint(options, 1, " done.\n");
    myPrint(options, 2, "Runtime: ", e, "s \n");
}
