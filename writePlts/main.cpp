#include "AMReX_MultiFab.H"
#include "AMReX_Print.H"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include <AMReX.H>
#include <AMReX_ParmParse.H>
#include <AMReX_VisMF.H>
#include <AMReX_Geometry.H>
#include "AMReX_PlotFileUtil.H"


using namespace std;
using namespace amrex;



// Read the boxCounts file indicating the number of boxes at each time step
vector<float> readBoxCounts (string dir) {

    vector<float> boxCounts;

    fstream fout;

    fout.open((dir + "boxCounts.raw"), ios::in | ios::binary);
    if (!fout.is_open()) {
        cerr << "Error opening file: " << dir + "boxCounts.raw" << endl;
        return {};  // Return empty vector on failure
    }
    while (true) {
        float count;
        fout.read(reinterpret_cast<char*>(&count), sizeof(float));
        if (fout.gcount() < 4) {
            break;
        }
        boxCounts.push_back(count);
    }
    fout.close();

    boxCounts.shrink_to_fit();

    return boxCounts;

}


// Read the location and dimension files indicating the location and dimension of each box
vector<tuple<vector<vector<float>>, vector<vector<float>>>> readLocDim (string dir, vector<float> boxCounts) {

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
            // Print() << "Box loc: " << to_string(location[0]) << " " << to_string(location[1]) << " " << to_string(location[2]) << endl;
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


// create a multifab with the apropriate locations and dimensions
MultiFab createMF (vector<vector<float>> locations, vector<vector<float>> dimensions, int numBoxes) {

    BoxList boxes;

    for (int i=0; i < numBoxes; i++) {

        vector<float> loc = locations[i];
        vector<float> dim = dimensions[i];
        IntVect lo(static_cast<int>(loc[0]), static_cast<int>(loc[1]), static_cast<int>(loc[2]));
        IntVect hi(static_cast<int>(loc[0] + dim[0] - 1), static_cast<int>(loc[1] + dim[1] - 1), static_cast<int>(loc[2] + dim[2] - 1));
        // Print() << "Box " << i << ": Location = (" << loc[0] << ", " << loc[1] << ", " << loc[2] << "), Dimension = ("
        //         << dim[0] << ", " << dim[1] << ", " << dim[2] << ")" << endl;

        Box current(lo, hi);
        boxes.push_back(current);
    }

    if (boxes.size() != numBoxes) {
        cerr << "Error: Number of boxes in BoxList does not match the expected count." << endl;
    }

    BoxArray array(boxes);
    DistributionMapping dm(array);
    int ncomp = 1;
    int ngrow = 0;

    MultiFab output(array, dm, ncomp, ngrow);

    return output;

}


// populate a multifab with data
void populateMF (MultiFab& multi, const string& dir, int time) {

    int box_idx = 0;

    for (MFIter mfi(multi, false); mfi.isValid(); ++mfi) {

        const Box& box = mfi.validbox();
        const Array4<Real>& mfdata = multi.array(mfi);
        const auto lo = lbound(box);
        const auto hi = ubound(box);

        string filename = dir + "decodedBox-" + to_string(time) + "-" + to_string(box_idx) + ".raw";
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

    }

}



int main (int argc, char* argv[]) {

    Initialize(argc, argv);

    ParmParse pp;
    string encodedDir;
    string decodedDir;
    string out;
    int nlevels;
    int numTimes;
    int xDim;
    int yDim;
    int zDim;

    pp.query("encodedDir", encodedDir);
    pp.query("decodedDir", decodedDir);
    pp.query("out", out);
    pp.query("levels", nlevels);
    pp.query("timesteps", numTimes);
    pp.query("xDim", xDim);
    pp.query("yDim", yDim);
    pp.query("zDim", zDim);


    for (int i=0; i < numTimes; i++) {

        const string name = out + Concatenate("plt", i+74);
        Vector<MultiFab> mfs;
        const Vector<string> varnames = {"temp"};
        Vector<Geometry> geoms;
        Real time = 1.0;
        Vector<int> level_steps;
        Vector<IntVect> ref_ratio;

        for(int l=0; l < nlevels; l++) {

            string encoded = encodedDir + to_string(l) + "/";
            string decoded = decodedDir + to_string(l) + "/";

            vector<float> boxPerTime;
            boxPerTime = readBoxCounts(encoded);

            vector<tuple<vector<vector<float>>, vector<vector<float>>>> locdim;
            locdim = readLocDim(encoded, boxPerTime);

            tuple<vector<vector<float>>, vector<vector<float>>> locdimCurrent = locdim[i];
            vector<vector<float>> locations = get<0>(locdimCurrent);
            vector<vector<float>> dimensions = get<1>(locdimCurrent);
            int numBoxes = static_cast<int>(boxPerTime[i]);
            Print() << "Number of boxes at timestep " << to_string(i) << ", level " << to_string(l) << " = " << to_string(numBoxes) << endl;
            // Print() << "Number of locations stored at timestep " << to_string(i) << ", level " << to_string(l) << " = " << to_string(locations.size()) << endl;
            // Print() << "Number of dimensions stored at timestep " << to_string(i) << ", level " << to_string(l) << " = " << to_string(dimensions.size()) << endl;

            if (numBoxes == locations.size() && numBoxes == dimensions.size()) {
                Print() << "Locations, dimensions read successfully." << endl;
            } else {
                Print() << "Error: number of stored locations or dimensions does not match number of boxes!" << endl;
                break;
            }

            MultiFab mf = createMF(locations, dimensions, numBoxes);
            // Print() << "Successfully created MultiFab with " << to_string(mf.size()) << " boxes." << endl;
            populateMF(mf, decoded, i);
            // Print() << "Successfully populated MultiFab with " << to_string(mf.size()) << " boxes." << endl;
            mfs.push_back(std::move(mf));

            int xDimCurrent = xDim * pow(2, l);
            int yDimCurrent = yDim * pow(2, l);
            int zDimCurrent = zDim * pow(2, l);
            Print() << "xDim: " << xDimCurrent << ", yDim: " << yDimCurrent << ", zDim: " << zDimCurrent << endl;

            Box domain(IntVect(0, 0, 0), IntVect(xDimCurrent-1, yDimCurrent-1, zDimCurrent-1));
            RealBox cell({0.0, 0.0, 0.0,}, {static_cast<double>(xDim), static_cast<double>(yDim), static_cast<double>(zDim)});
            Array<int,AMREX_SPACEDIM> is_periodic {AMREX_D_DECL(0, 0, 0)};
            const Geometry geom(domain, cell, 0, is_periodic);
            geoms.push_back(geom);
            int level_step = 1;
            level_steps.push_back(level_step);
            if (l > 0) {
                IntVect ratio(2, 2, 2);
                ref_ratio.push_back(ratio);
            }
            // Print() << "Successfully assigned variables for plotfile." << endl;
        }

        Vector<const MultiFab*> mfPtrs;
        for (auto& mf : mfs) {
            mfPtrs.push_back(&mf);
        }
        const Vector<const MultiFab*> constMfs = mfPtrs;
        const Vector<Geometry> constGeoms = geoms;
        const Vector<int> constLevelSteps = level_steps;
        const Vector<IntVect> constRefRatio = ref_ratio;
        // Print() << "Ready to write plotfile." << endl;

        WriteMultiLevelPlotfile(name, nlevels, constMfs, varnames, constGeoms, time, constLevelSteps, constRefRatio);
        Print() << "Successfully wrote plotfile." << endl;

    }

    Finalize();

}
