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
  size_t anzahlStreckenelemente;  // inkl. nicht belegte Nummern (0-indiziert)
  size_t anzahlRegisterAutomatisch;  // inkl. nicht belegte Nummern (5000-indiziert)
  size_t anzahlRegisterManuell;  // inkl. nicht belegte Nummern (1-indiziert)
};

int main(int argc, char** argv) {
  boost::nowide::args a(argc, argv);
  std::string fpn_datei;
  size_t element_nr;

  po::options_description desc("Kommandozeilenparameter");
  desc.add_options()
    ("help", "Hilfe zur Programmbenutzung anzeigen")
    ("fpn", po::value<std::string>(&fpn_datei), "Zu ladende Fahrplandatei")
    ("element-nr", po::value<size_t>(&element_nr), "Elementnummer in der zusammengesetzten Strecke");

  po::positional_options_description p;
  p.add("fpn", 1);
  p.add("element-nr", 1);

  po::variables_map vars;
  try {
    po::store(po::command_line_parser(argc, argv). options(desc).positional(p).run(), vars);
  } catch (const boost::program_options::error& e) {
    boost::nowide::cout << e.what() << "\n";
    return 1;
  }
  po::notify(vars);

  if (vars.count("help") || !vars.count("fpn") || !vars.count("element-nr")) {
    boost::nowide::cout << argv[0] << " [FPN-DATEI] [ELEMENTNUMMER]\n\n";
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

  const auto fpn_datei_zusi_pfad = zusixml::ZusiPfad::vonOsPfad(fpn_datei);
  std::vector<ModulInfo> modulInfo;
  for (const auto& modul : fpn->children_StrModul) {
    if (!modul) {
      continue;
    }
    const auto& st3 = zusixml::parseFile(zusixml::ZusiPfad::vonZusiPfad(modul->Datei.Dateiname, fpn_datei_zusi_pfad).alsOsPfad());
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
          if (reg < 5000) {
            max_reg_manuell = std::max(max_reg_manuell, static_cast<size_t>(reg));
          } else {
            max_reg_automatisch = std::max(max_reg_automatisch, static_cast<size_t>(reg));
          }
        }
      }
      if (element->InfoGegenRichtung) {
        if (const auto& reg = element->InfoGegenRichtung->Reg; reg > 0) {
          if (reg < 5000) {
            max_reg_manuell = std::max(max_reg_manuell, static_cast<size_t>(reg));
          } else {
            max_reg_automatisch = std::max(max_reg_automatisch, static_cast<size_t>(reg));
          }
        }
      }
    }

    modulInfo.emplace_back(ModulInfo {
      modul->Datei.Dateiname,
      st3->Strecke->children_StrElement.size(),
      max_reg_automatisch > 0 ? max_reg_automatisch - 5000 + 1 : 0,
      max_reg_manuell > 0 ? max_reg_manuell : 0
    });
  }

  size_t element_nr_start = 1;
  size_t reg_manuell_nr_start = 1;
  size_t reg_automatisch_nr_start = 5001;
  for (const auto& modul : modulInfo) {
#ifndef NDEBUG
    boost::nowide::cerr << "---- " << modul.dateiname << " ----\n";
#endif
    if (modul.anzahlStreckenelemente > 0) {
#ifndef NDEBUG
      boost::nowide::cerr << " - " << "Streckenelemente: [0, " << (0 + modul.anzahlStreckenelemente - 1) << "] -> [" << element_nr_start << ", " << (element_nr_start + modul.anzahlStreckenelemente - 1) << "]\n";
#endif
      if (element_nr >= element_nr_start && element_nr < element_nr_start + modul.anzahlStreckenelemente) {
        boost::nowide::cout << "Streckenelement Nr. " << element_nr << ": " << modul.dateiname << ", Streckenelement Nr. " << (0 + element_nr - element_nr_start) << "\n";
      }
      element_nr_start += modul.anzahlStreckenelemente;
    }

    if (modul.anzahlRegisterManuell > 0) {
#ifndef NDEBUG
      boost::nowide::cerr << " - " << "Manuelle Register: [1, " << (1 + modul.anzahlRegisterManuell - 1) << "] -> [" << reg_manuell_nr_start << ", " << (reg_manuell_nr_start + modul.anzahlRegisterManuell - 1) << "]\n";
#endif
      if (element_nr >= reg_manuell_nr_start && element_nr < reg_manuell_nr_start + modul.anzahlRegisterManuell) {
        boost::nowide::cout << "(Manuelles) Register Nr. " << element_nr << ": " << modul.dateiname << ", Register Nr. " << (1 + element_nr - reg_manuell_nr_start) << "\n";
      }
      reg_manuell_nr_start += modul.anzahlRegisterManuell;
    }

    if (modul.anzahlRegisterAutomatisch > 0) {
#ifndef NDEBUG
      boost::nowide::cerr << " - " << "Automatische Register: [5000, " << (5000 + modul.anzahlRegisterAutomatisch - 1) << "] -> [" << reg_automatisch_nr_start << ", " << (reg_automatisch_nr_start + modul.anzahlRegisterAutomatisch - 1) << "]\n";
#endif
      if (element_nr >= reg_automatisch_nr_start && element_nr < reg_automatisch_nr_start + modul.anzahlRegisterAutomatisch) {
        boost::nowide::cout << "(Automatisches) Register Nr. " << element_nr << ": " << modul.dateiname << ", Register Nr. " << (5000 + element_nr - reg_automatisch_nr_start) << "\n";
      }

      reg_automatisch_nr_start += modul.anzahlRegisterAutomatisch;
    }
  }
}
