////////////////////////////////////////////////////////////////////////////////

// /* GEANT4 code for propagation of gamma-rays, electron and positrons in
// Earth's atmosphere */
//
// //
// // ********************************************************************
// // * License and Disclaimer                                           *
// // *                                                                  *
// // * The  Geant4 software  is  copyright of the Copyright Holders  of *
// // * the Geant4 Collaboration.  It is provided  under  the terms  and *
// // * conditions of the Geant4 Software License,  included in the file *
// // * LICENSE and available at  http://cern.ch/geant4/license .  These *
// // * include a list of copyright holders.                             *
// // *                                                                  *
// // * Neither the authors of this software system, nor their employing *
// // * institutes,nor the agencies providing financial support for this *
// // * work  make  any representation or  warranty, express or implied, *
// // * regarding  this  software system or assume any liability for its *
// // * use.  Please see the license in the file  LICENSE  and URL above *
// // * for the full disclaimer and the limitation of liability.         *
// // *                                                                  *
// // * This  code  implementation is the result of  the  scientific and *
// // * technical work of the GEANT4 collaboration.                      *
// // * By using,  copying,  modifying or  distributing the software (or *
// // * any work based  on the software)  you  agree  to acknowledge its *
// // * use  in  resulting  scientific  publications,  and indicate your *
// // * acceptance of all terms of the Geant4 Software license.          *
// // ********************************************************************
////////////////////////////////////////////////////////////////////////////////

#include <Analysis.hh>
#include <Settings.hh>
#include <fstream>
#include <iomanip>

#include "G4SteppingManager.hh"
#include "G4Track.hh"
#include "myUtils.hh"

#include <thread>
#include <chrono>

// class following singleton pattern

Analysis *Analysis::instance = nullptr;

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

Analysis *Analysis::getInstance() // singleton lazy initialization
{
    if (instance == nullptr) {
        instance = new Analysis;
    }

    return instance;
}

// constructor
Analysis::Analysis() {

    const long unique_ID = myUtils::generate_a_unique_ID();

    const double ALT_MAX_RECORDED = Settings::record_altitude;

    filename_unique_ID = unique_ID;

    //
    const G4String output_filename_second_part =
            std::to_string(filename_unique_ID) + "_" +
            std::to_string(int(ALT_MAX_RECORDED)) + "_" +
            std::to_string(int(Settings::SOURCE_ALT)) + "_" +
            std::to_string(int(Settings::SOURCE_OPENING_ANGLE)) + "_" +
            Settings::BEAMING_TYPE + "_" +
            std::to_string(int(Settings::SOURCE_SIGMA_TIME)) + ".out";
    //

    if ((Settings::BEAMING_TYPE == "Uniform") ||
        (Settings::BEAMING_TYPE == "uniform")) {
        number_beaming = 0;
    } else if ((Settings::BEAMING_TYPE == "Gaussian") ||
               (Settings::BEAMING_TYPE == "gaussian") ||
               (Settings::BEAMING_TYPE == "normal") ||
               (Settings::BEAMING_TYPE == "Normal")) {
        number_beaming = 1;
    }

    ///

    output_lines.clear();

    mkdir("./output_ascii/",0777);

    asciiFileName2 = "./output_ascii/detParticles_" + output_filename_second_part;
    std::ofstream asciiFile00(asciiFileName2,
                              std::ios::trunc); // to clean the output file
    asciiFile00.close();
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4int Analysis::get_NB_RECORDED() const { return NB_RECORDED; }

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

Analysis::~Analysis() = default;

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void Analysis::save_in_output_buffer(
        const G4int PDG_NB, const G4double &time, const G4double &energy,
        const G4double &dist_rad, const G4int ID, const G4double &ecef_x,
        const G4double &ecef_y, const G4double &ecef_z, const G4double &mom_x,
        const G4double &mom_y, const G4double &mom_z, const G4double &lat,
        const G4double &lon, const G4double &alt, const int event_nb) {

    //
    double alt2 = alt / 1000.0; // m to km

    bool record_or_not = true;

    // ASCII OUTPUT
    if (Settings::OUTPUT_TO_ASCII_FILE) {
        std::stringstream buffer;
        buffer << std::scientific
               << std::setprecision(5); // scientific notation with
        // 5 significant digits
        //   asciiFile << name;
        //   asciiFile << ' ';
        buffer << filename_unique_ID; //1
        buffer << ' ';
        buffer << Settings::SOURCE_ALT; //2
        buffer << ' ';
        buffer << Settings::SOURCE_OPENING_ANGLE; //3
        buffer << ' ';
        buffer << event_nb; // 4
        buffer << ' ';
        buffer << time; //5
        buffer << ' ';
        buffer << energy; //6
        buffer << ' ';
        buffer << lat; //7
        buffer << ' ';
        buffer << lon; //8
        buffer << ' ';
        buffer << Settings::record_altitude; //9
        buffer << ' ';
        buffer << mom_x; //10
        buffer << ' ';
        buffer << mom_y; //11
        buffer << ' ';
        buffer << mom_z; //12
        buffer << G4endl;
        //
        NB_RECORDED++;
        //

        output_lines.push_back(buffer.str());
        //
        write_in_output_file();
    }

}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
void Analysis::write_in_output_file() {

    if (output_lines.size() <= output_buffer_size) {
        return;
    }

    std::ofstream asciiFile2;
    asciiFile2.open(asciiFileName2, std::ios::app);

    if (asciiFile2.is_open()) {
        for (const G4String &line : output_lines) {
            asciiFile2 << line;
        }

        asciiFile2.close();
        output_lines.clear();
    }
}

// ....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
void Analysis::write_in_output_file_endOfRun() {
    if (output_lines.empty()) {
        return;
    }

    std::ofstream asciiFile1;
    asciiFile1.open(asciiFileName2, std::ios::app);

    if (asciiFile1.is_open()) {
        for (G4String &line : output_lines) {
            asciiFile1 << line;
        }

        asciiFile1.close();
        output_lines.clear();
    }
}