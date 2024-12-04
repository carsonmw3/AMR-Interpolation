#include "AMReX_MultiFab.H"
#include "AMReX_Print.H"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <AMReX.H>
#include <AMReX_ParmParse.H>
#include <AMReX_VisMF.H>
#include <AMReX_Geometry.H>
#include "AMReX_PlotFileUtil.H"



using namespace std;
using namespace amrex;

vector<int> readBoxCounts (string dir) {

    vector<int> boxCounts;

    fstream fout;

    Print() << "Trying to open: " << dir + "boxCounts.raw" << endl;
    fout.open((dir + "boxCounts.raw"), ios::in | ios::binary);
    if (!fout.is_open()) {
        cerr << "Error opening file: " << dir + "boxCounts.raw" << endl;
        return {};  // Return empty vector on failure
    }
    while (true) {
        int count;
        fout.read(reinterpret_cast<char*>(&count), sizeof(int));
        if (fout.gcount() < 4) {
            break;
        }
        Print() << "Box count: " << to_string(count) << endl;
        boxCounts.push_back(count);
    }
    fout.close();

    boxCounts.shrink_to_fit();

    return boxCounts;

}


vector<tuple<vector<vector<float>>, vector<vector<float>>>> readLocDim (string dir, vector<int> boxCounts) {

    fstream fout;

    vector<tuple<vector<vector<float>>, vector<vector<float>>>> output;

    for(int i=0; i < boxCounts.size(); i++) {

        int boxCount = static_cast<int>(boxCounts[i]);
        vector<vector<float>> locations;
        vector<vector<float>> dimensions;

        // read locations
        fout.open((dir + "locations.raw"), ios::in | ios::binary);
        for (int i=0; i < boxCount; i++) {
            vector<float> location(3);
            fout.read(reinterpret_cast<char*>(&location[0]), sizeof(float));
            fout.read(reinterpret_cast<char*>(&location[1]), sizeof(float));
            fout.read(reinterpret_cast<char*>(&location[2]), sizeof(float));
            locations.push_back(location);
        }
        fout.close();

        // read dimensions
        fout.open((dir + "dimensions.raw"), ios::in | ios::binary);
        for (int i=0; i < boxCount; i++) {
            vector<float> dimension(3);
            fout.read(reinterpret_cast<char*>(&dimension[0]), sizeof(float));
            fout.read(reinterpret_cast<char*>(&dimension[1]), sizeof(float));
            fout.read(reinterpret_cast<char*>(&dimension[2]), sizeof(float));
            dimensions.push_back(dimension);
        }
        fout.close();

        locations.shrink_to_fit();
        dimensions.shrink_to_fit();
        tuple<vector<vector<float>>, vector<vector<float>>> tup(locations, dimensions);
        output.push_back(tup);

    }

    return output;

}


MultiFab createMF (vector<vector<float>> locations, vector<vector<float>> dimensions, int numBoxes) {

    BoxList boxes;

    for (int i=0; i < numBoxes; i++) {

        vector<float> loc = locations[i];
        vector<float> dim = dimensions[i];
        IntVect lo(static_cast<int>(loc[0]), static_cast<int>(loc[1]), static_cast<int>(loc[2]));
        IntVect hi(static_cast<int>(loc[0] + dim[0] - 1), static_cast<int>(loc[1] + dim[1] - 1), static_cast<int>(loc[2] + dim[2] - 1));
        Box current(lo, hi);
        boxes.push_back(current);
    }

    BoxArray array(boxes);
    DistributionMapping dm(array);
    int ncomp = 1;
    int ngrow = 0;

    MultiFab output(array, dm, ncomp, ngrow);

    return output;

}


int populateMF (MultiFab& multi, const string& dir, int box_idx) {

    for (MFIter mfi(multi, false); mfi.isValid(); ++mfi) {

        const Box& box = mfi.validbox();
        const Array4<Real>& mfdata = multi.array(mfi);
        const auto lo = lbound(box);
        const auto hi = ubound(box);

        string filename = dir + "decodedBox-" + to_string(box_idx) + ".raw";
        fstream fout;
        fout.open(filename, ios::in | ios::binary);

        for (int k = lo.z; k <= hi.z; k++) {

            for (int j = lo.y; j <= hi.y; j++) {

                for (int i = lo.x; i <= hi.x; i++) {

                    fout.read(reinterpret_cast<char*>(&mfdata(i,j,k,0)), sizeof(float));

                }
            }
        }

        fout.close();
        box_idx += 1;

    }

    return box_idx;

}



int main (int argc, char* argv[]) {

    Initialize(argc, argv);

    ParmParse pp;
    string encodedDir;
    string decodedDir;
    string out;

    pp.query("encodedDir", encodedDir);
    pp.query("decodedDir", decodedDir);
    pp.query("out", out);

    vector<int> boxPerTime;
    boxPerTime = readBoxCounts(encodedDir);
    int numTimes = boxPerTime.size();
    Print() << "Timesteps: " << to_string(numTimes) << endl;

    vector<tuple<vector<vector<float>>, vector<vector<float>>>> locdim;
    locdim = readLocDim(encodedDir, boxPerTime);

    int box_idx = 0;

    for (int i=0; i < numTimes; i++) {

        tuple<vector<vector<float>>, vector<vector<float>>> locdimCurrent = locdim[i];
        vector<vector<float>> locations = get<0>(locdimCurrent);
        vector<vector<float>> dimensions = get<1>(locdimCurrent);
        int numBoxes = boxPerTime[i];
        Print() << "Number of boxes at timestep " << to_string(i) << "= " << to_string(numBoxes) << endl;

        MultiFab mf = createMF(locations, dimensions, numBoxes);
        box_idx = populateMF(mf, decodedDir, box_idx);

        // Implement ways to automate these definitions
        const string name = out + Concatenate("plt", i);
        const Vector<string> varnames = {"temp"};
        Box domain(IntVect(0, 0, 0), IntVect(512, 192, 192));
        RealBox cell({0.0, 0.0, 0.0,}, {1.0, 1.0, 1.0});
        Array<int,AMREX_SPACEDIM> is_periodic {AMREX_D_DECL(0, 0, 0)};
        const Geometry geom(domain, cell, 0, is_periodic);
        Real time = 1;
        int level_step = 1;

        WriteSingleLevelPlotfile(name, mf, varnames, geom, time, level_step);

    }

}
