#include "zusi_parser/zusi_types.hpp"
#include "zusi_parser/zusi_parser.hpp"
#include "zusi_parser/utils.hpp"

#include <boost/program_options.hpp>
#include <boost/nowide/args.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/fstream.hpp>

#include <unordered_map>
#include <unordered_set>
#include <optional>

#ifdef WIN32
#define UNICODE
#endif

namespace po = boost::program_options;

struct ModulInfo {
  std::string dateiname;
  size_t maxElementNr;
  size_t maxRegisterNrAutomatisch;
  size_t maxRegisterNrManuell;
  size_t maxReferenzNr;
};

int main(int argc, char** argv) {
  boost::nowide::args a(argc, argv);
  std::string fpn_datei;
  size_t element_nr;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("fpn", po::value<std::string>(&fpn_datei), "Zu ladende Fahrplandatei")
    ("element-nr", po::value<size_t>(&element_nr), "Elementnummer in der zusammengesetzten Strecke");

  po::positional_options_description p;
  p.add("fpn", -1);
  p.add("element-nr", -1);

  po::variables_map vars;
  po::store(po::command_line_parser(argc, argv). options(desc).positional(p).run(), vars);
  po::notify(vars);

  if (vars.count("help")) {
    boost::nowide::cout << desc << "\n";
    return 1;
  }

  std::unique_ptr<const Fahrplan> fpn;
  try {
    const auto& fpn_parsed = zusixml::parseFile(fpn_datei);
    if (!fpn_parsed || !fpn_parsed->Fahrplan) {
      boost::nowide::cerr << "Keine gueltige Fahrplandatei: " << fpn_datei << "\n";
      exit(1);
    }
    fpn = std::move(fpn_parsed->Fahrplan);
  } catch (const std::runtime_error& e) {
    boost::nowide::cerr << e.what() << "\n";
    exit(1);
  }

  std::vector<ModulInfo> modulInfo;
  for (const auto& modul : fpn->children_StrModul) {
    if (!modul) {
      continue;
    }
    const auto& st3 = zusixml::parseFile(zusixml::zusiPfadZuOsPfad(modul->Datei.Dateiname, fpn_datei));
    if (!st3 || !st3->Strecke) {
      continue;
    }

    size_t max_reg_automatisch = 0;
    size_t max_reg_manuell = 0;
    for (const auto& element : st3->Strecke->children_StrElement) {
      if (!element) {
        continue;
      }

      if (element->InfoNormRichtung) {
        if (const auto& reg = element->InfoNormRichtung->Reg; reg > 0) {
          if (reg < 1000) {
            max_reg_manuell = reg;
          } else {
            max_reg_automatisch = reg;
          }
        }
      }
      if (element->InfoGegenRichtung) {
        if (const auto& reg = element->InfoGegenRichtung->Reg; reg > 0) {
          if (reg < 1000) {
            max_reg_manuell = reg;
          } else {
            max_reg_automatisch = reg;
          }
        }
      }
    }

    modulInfo.emplace_back(ModulInfo {
      modul->Datei.Dateiname,
      st3->Strecke->children_StrElement.size(),
      max_reg_automatisch,
      max_reg_manuell,
      st3->Strecke->children_ReferenzElemente.size()
    });
  }

  size_t element_nr_start = 0;
  size_t reg_manuell_nr_start = 0;
  size_t reg_automatisch_nr_start = 0;
  size_t referenz_nr_start = 0;
  for (const auto& modul : modulInfo) {
    if (element_nr > element_nr_start && element_nr < element_nr_start + modul.maxElementNr) {
      boost::nowide::cout << "Streckenelement Nr. " << element_nr << ": " << modul.dateiname << ", Element " << (element_nr - element_nr_start - 1) << "\n";
    }
    if (element_nr > reg_manuell_nr_start && element_nr < reg_manuell_nr_start + modul.maxElementNr) {
      boost::nowide::cout << "Register Nr. " << element_nr << ": " << modul.dateiname << ", Element " << (element_nr - reg_manuell_nr_start - 1) << "\n";
    }
    if (element_nr > reg_automatisch_nr_start && element_nr < reg_automatisch_nr_start + modul.maxElementNr) {
      boost::nowide::cout << "Register Nr. " << element_nr << ": " << modul.dateiname << ", Element " << (element_nr - reg_automatisch_nr_start - 1) << "\n";
    }
    if (element_nr > referenz_nr_start && element_nr < referenz_nr_start + modul.maxElementNr) {
      boost::nowide::cout << "Referenzpunkt Nr. " << element_nr << ": " << modul.dateiname << ", Element " << (element_nr - referenz_nr_start - 1) << "\n";
    }

    element_nr_start += modul.maxElementNr;
    reg_manuell_nr_start += modul.maxRegisterNrManuell;
    reg_automatisch_nr_start += modul.maxRegisterNrAutomatisch;
    referenz_nr_start += modul.maxReferenzNr;
  }
}
