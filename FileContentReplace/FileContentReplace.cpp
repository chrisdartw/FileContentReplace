//
// FileContentReplace.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/convert.hpp>
#include <boost/convert/stream.hpp>

namespace bFS = boost::filesystem;
int FuncCopy (bFS::path me, bFS::path bios, bFS::path output, int position = -1, bool verbose = false) throw()
{
  if ( (false == bFS::exists (me)) || (false == bFS::exists (bios))) {
    std::cout << "ERROR: File does not exist (me or bios)" << std::endl << std::endl;
    return EXIT_FAILURE;
  }
  if (bFS::file_size (me) <= bFS::file_size (bios)) {
    std::cout << "ERROR: File size is not legal (me <= bios)" << std::endl << std::endl;
    return EXIT_FAILURE;
  }
  if ( (-1 != position) && (bFS::file_size (me) < (position + bFS::file_size (bios)))) {
    std::cout << "ERROR: position is not legal (me < position + bios)" << std::endl << std::endl;
    return EXIT_FAILURE;
  }
  if (true == verbose) {
    std::cout << boost::format ("me     \n size : 0x%1$08x\n name : %2%") % bFS::file_size (me) % me.string() << std::endl;
    std::cout << boost::format ("bios   \n size : 0x%1$08x\n name : %2%") % bFS::file_size (bios) % bios.string() << std::endl;
    std::cout << boost::format ("output \n size : 0x%1$08x\n name : %2%") % bFS::file_size (me) % output.string() << std::endl;
  } else {
    std::cout << boost::format ("Processing for %1% ... ") % output.filename().string();
  }

  // Phase 1 : Read me file.
  std::ifstream File_me (me.string(), std::ios::binary);
  std::vector<char> DataBuffer ( (std::istreambuf_iterator<char> (File_me)), std::istreambuf_iterator<char>());
  File_me.close();

  // Phase 2 : Read bios file in the correct position.
  auto Offset_Iter = std::next (DataBuffer.begin(), size_t (bFS::file_size (me) - bFS::file_size (bios)));
  Offset_Iter = (-1 != position ? std::next (DataBuffer.begin(), position) : Offset_Iter);
  std::ifstream File_bios (bios.string(), std::ios::binary);
  std::copy (std::istreambuf_iterator<char> (File_bios), std::istreambuf_iterator<char>(), Offset_Iter);
  File_bios.close();

  // Phase 3 : Write output file.
  std::ofstream File_output (output.string(), std::ios::binary);
  if (File_output.good()) {
    std::copy (DataBuffer.begin(), DataBuffer.end(), std::ostreambuf_iterator<char> (File_output));
  } else {
    if (true == verbose) {
      std::cout << "Output file write fail." << std::endl;
    } else {
      std::cout << "Fail." << std::endl;
    }
    return EXIT_FAILURE;
  }
  File_output.close();

  std::cout << "Done." << std::endl;
  return EXIT_SUCCESS;
}

int FuncSwap (bFS::path input, size_t size, bFS::path output, bool verbose = false) throw()
{
  if (false == bFS::exists (input)) {
    std::cout << "ERROR: File does not exist (input)" << std::endl << std::endl;
    return EXIT_FAILURE;
  }

  enum FileSizeIdx {
    KiloBytes = 1024,
    Size_64KB = 64 * KiloBytes,
    Size_128KB = 128 * KiloBytes,
    Size_256KB = 256 * KiloBytes,
    Size_512KB = 512 * KiloBytes,
    Size_1MB = 1024 * KiloBytes,
  };
  switch (size * KiloBytes) {
  case Size_64KB:
  case Size_128KB:
  case Size_256KB:
  case Size_512KB:
  case Size_1MB:
    break;
  default:
    std::cout << "ERROR: The size must be one of 64, 128, 256, 512, 1024" << std::endl << std::endl;
    return EXIT_FAILURE;
  }
  if ( ((uint64_t)2 * size * KiloBytes) >= bFS::file_size (input)) {
    std::cout << boost::format ("ERROR: File size is not legal (%1%KB >= input)") % (2 * size) << std::endl << std::endl;
    return EXIT_FAILURE;
  }
  if (true == verbose) {
    std::cout << boost::format ("input  \n size : 0x%1$08x\n name : %2%") % bFS::file_size (input) % input.string() << std::endl;
    std::cout << boost::format ("output \n size : 0x%1$08x\n name : %2%") % bFS::file_size (input) % output.string() << std::endl;
  } else {
    std::cout << boost::format ("Processing for %1% ... ") % output.filename().string();
  }

  // Phase 1 : Read input file.
  std::ifstream File_input (input.string(), std::ios::binary);
  std::vector<char> DataBuffer ( (std::istreambuf_iterator<char> (File_input)), std::istreambuf_iterator<char>());
  File_input.close();

  // Phase 2 : File content check.
  auto Start_Iter = std::next (DataBuffer.begin(), size_t (bFS::file_size (input) - (uint64_t)2 * size * KiloBytes));
  auto Stop_Iter = std::next (DataBuffer.begin(), size_t (bFS::file_size (input) - (uint64_t)1 * size * KiloBytes));
  if (1 != std::set<char> (Start_Iter, Stop_Iter).size()) {
    if (true == verbose) {
      std::cout << "Input file content check fail." << std::endl;
    } else {
      std::cout << "Fail." << std::endl;
    }
    return EXIT_FAILURE;
  }

  // Phase 3 : Copy content to the correct position.
  std::copy (Stop_Iter, DataBuffer.end(), Start_Iter);

  // Phase 4 : Write output file.
  std::ofstream File_output (output.string(), std::ios::binary);
  if (File_output.good()) {
    std::copy (DataBuffer.begin(), DataBuffer.end(), std::ostreambuf_iterator<char> (File_output));
  } else {
    if (true == verbose) {
      std::cout << "Output file write fail." << std::endl;
    } else {
      std::cout << "Fail." << std::endl;
    }
    return EXIT_FAILURE;
  }
  File_output.close();

  std::cout << "Done." << std::endl;
  return EXIT_SUCCESS;
}

namespace bPO = boost::program_options;
typedef boost::tuple<bPO::options_description, bPO::variables_map, std::string> MyProgramOptions;
enum MyProgramOptionsTupleIdx {
  MPO_Desc = 0,
  MPO_Value,
  MPO_Error,
  MPO_Max,
};
typedef std::vector<MyProgramOptions> myOptionContainer;

int _tmain (int argc, _TCHAR *argv[]) throw()
{
  enum FunctionIdx {
    FunctionHelp = 0,
    FunctionCopy,
    FunctionSwap,
    FunctionMax,
  };

  // Phase 1 : Set program options.
  myOptionContainer OptionGroup;
  OptionGroup.push_back (MyProgramOptions{ bPO::options_description ("Help") });
  OptionGroup.push_back (MyProgramOptions{ bPO::options_description ("BIOS Replace") });
  OptionGroup.push_back (MyProgramOptions{ bPO::options_description ("Top Copy") });

  OptionGroup[FunctionHelp].get<MPO_Desc>().add_options()
  ("help,h", "Print help messages")
  ("verbose,v", "Print verbose messages");

  OptionGroup[FunctionCopy].get<MPO_Desc>().add_options()
  ("me,m", bPO::value<std::string>()->value_name ("filename")->required(), "[Required] Whole ROM image included ME & BIOS")
  ("bios,b", bPO::value<std::string>()->value_name ("filename")->required(), "[Required] BIOS file only, For replacement purposes")
  ("output,o", bPO::value<std::string>()->value_name ("filename")->required(), "[Required] output file")
  ("position,p", bPO::value<std::string>()->value_name ("position")->required(), "[Required] Paste position, -1 for tail");

  OptionGroup[FunctionSwap].get<MPO_Desc>().add_options()
  ("input,i", bPO::value<std::string>()->value_name ("filename")->required(), "[Required] input file")
  ("size,s", bPO::value<size_t>()->value_name ("size")->required(), "[Required] replace size")
  ("output,o", bPO::value<std::string>()->value_name ("filename")->required(), "[Required] output file");

  // Phase 2 : Analysis program options.
  for (auto &item : OptionGroup) {
    try {
      namespace bCLS = boost::program_options::command_line_style;
      int style = bCLS::default_style | bCLS::case_insensitive | bCLS::allow_slash_for_short | bCLS::allow_long_disguise;
#if 0
      // deny_unregistered
      bPO::store (bPO::parse_command_line (argc, argv, item.get<MPO_Desc>(), style), item.get<MPO_Value>());
#else
      // allow_unregistered
      bPO::store (bPO::wcommand_line_parser (argc, argv).options (item.get<MPO_Desc>()).allow_unregistered().style (style).run(), item.get<MPO_Value>());
#endif
      bPO::notify (item.get<MPO_Value>());
    } catch (bPO::validation_error e) {
      item.get<MPO_Error>() = std::string (e.what());
    } catch (bPO::error_with_option_name e) {
      item.get<MPO_Error>() = std::string (e.what());
    } catch (bPO::error e) {
      item.get<MPO_Error>() = std::string (e.what());
    }
  }
  size_t MatchCount = 0;
  for each (const auto & item in OptionGroup) {
    MatchCount += (item.get<MPO_Error>().size() == 0 ? 1 : 0);
  }

  // Phase 3 : Functional classification.
  bPO::options_description OptionsDescAll ("Allowed options");
  for each (const auto & item in OptionGroup) {
    OptionsDescAll.add (item.get<MPO_Desc>());
  }
  if (2 != MatchCount) {
    std::cout << OptionsDescAll << std::endl;
  } else if (0 != OptionGroup[FunctionHelp].get<MPO_Value>().count ("help")) {
    std::cout << OptionsDescAll << std::endl;
  } else if (0 == OptionGroup[FunctionCopy].get<MPO_Error>().size()) {
    auto StrPos = OptionGroup[FunctionCopy].get<MPO_Value>() ["position"].as<std::string>();
    int IntPos = -1;
    if (StrPos.compare (0, 2, "0x") == 0) {
      StrPos = StrPos.substr (2, std::string::npos);
      IntPos = boost::convert<int> (StrPos, boost::cnv::cstream() (std::hex) (std::skipws)).value_or (-1);
    } else {
      IntPos = boost::convert<int> (StrPos, boost::cnv::cstream() (std::dec) (std::skipws)).value_or (-1);
    }
    FuncCopy (
      OptionGroup[FunctionCopy].get<MPO_Value>() ["me"].as<std::string>(),
      OptionGroup[FunctionCopy].get<MPO_Value>() ["bios"].as<std::string>(),
      OptionGroup[FunctionCopy].get<MPO_Value>() ["output"].as<std::string>(),
      IntPos,
      (0 != OptionGroup[FunctionHelp].get<MPO_Value>().count ("verbose"))
    );
  } else if (0 == OptionGroup[FunctionSwap].get<MPO_Error>().size()) {
    FuncSwap (
      OptionGroup[FunctionSwap].get<MPO_Value>() ["input"].as<std::string>(),
      OptionGroup[FunctionSwap].get<MPO_Value>() ["size"].as<size_t>(),
      OptionGroup[FunctionSwap].get<MPO_Value>() ["output"].as<std::string>(),
      (0 != OptionGroup[FunctionHelp].get<MPO_Value>().count ("verbose"))
    );
  }

  // Phase 4 : Debugging information.
  if (false && (0 != OptionGroup[FunctionHelp].get<MPO_Value>().count ("verbose"))) {
    for each (const auto & item in OptionGroup) {
      for each (const auto & it in item.get<MPO_Value>()) {
        std::cout << it.first.c_str() << "=";
        auto &value = it.second.value();
        if (auto v = boost::any_cast<size_t> (&value)) {
          std::cout << *v << ",";
        } else if (auto v = boost::any_cast<std::string> (&value)) {
          std::cout << *v << ",";
        } else {
          std::cout << "error" << ",";
        }
      }
      std::cout << std::endl;
    }
    std::cout << MatchCount << std::endl;
  }

  return EXIT_SUCCESS;
}
//
// https://theboostcpplibraries.com/
// https://kheresy.wordpress.com/boostcpplibraries/
//
