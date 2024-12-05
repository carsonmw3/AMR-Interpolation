#include "AMReX_MultiFab.H"
#include "AMReX_Print.H"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include <AMReX.H>
#include <AMReX_ParmParse.H>
#include <AMReX_VisMF.H>


using namespace std;
using namespace amrex;


// Reads input data and puts it into one big vector representing the whole volume
vector<vector<vector<float>>> collectData (string lev_file, int lev, int component) {

    MultiFab mf;

    // read data into multifab
    VisMF::Read(mf, lev_file);

    // storage vector for extracted data
    int xDim = 256;
    int yDim = 96;
    int zDim = 96;
    for (int l = 0; l < lev; l++) {
        xDim = xDim * 2;
        yDim = yDim * 2;
        zDim = zDim * 2;
    }
    vector<vector<vector<float>>> extractedData(xDim, vector<vector<float>>(yDim, vector<float>(zDim)));

    for (MFIter mfi(mf, false); mfi.isValid(); ++mfi) {

        const Box& box = mfi.validbox();
        const Array4<Real>& mfdata = mf.array(mfi);
        //Print() << "Box size: " << box.size() << endl;
        const auto lo = lbound(box);
        //Print() << "Box loc: " << lo << endl;
        const auto hi = ubound(box);

        for (int k = lo.z; k <= hi.z; k++) {

            for (int j = lo.y; j <= hi.y; j++) {

                for (int i = lo.x; i <= hi.x; i++) {

                    extractedData[i][j][k] = static_cast<float>(mfdata(i,j,k,component));

                }
            }

        }

    }

    return extractedData;

}



vector<vector<float>> collectDataNewFormat (string lev_file, int lev, int component) {

    vector<vector<float>> extractedData;

    MultiFab mf;

    // read data into multifab
    VisMF::Read(mf, lev_file);
    BoxArray ba = mf.boxArray();

    for (MFIter mfi(mf, false); mfi.isValid(); ++mfi) {

        const Box& box = mfi.validbox();
        const Array4<Real>& mfdata = mf.array(mfi);
        const auto lo = lbound(box);
        const auto hi = ubound(box);
        const auto shape = box.size();
        const auto indexType = box.ixType();
        // Print() << "Box size: " << box.size() << endl;
        // Print() << "Box loc: " << lo << endl;
        // Print() << "Index type: " << indexType << endl;

        vector<float> boxData;

        boxData.push_back(lo.x);
        boxData.push_back(lo.y);
        boxData.push_back(lo.z);
        boxData.push_back(shape[0]);
        boxData.push_back(shape[1]);
        boxData.push_back(shape[2]);

        for (auto k = lo.z; k <= hi.z; ++k) {
            for (auto j = lo.y; j <= hi.y; ++j) {
                for (auto i = lo.x; i <= hi.x; ++i) {
                    boxData.push_back(mfdata(i,j,k,component));
                }
            }
        }

        extractedData.push_back(boxData);

        // for (int i=0; i < boxData.size(); i++) {
        //     Print() << boxData[i] << endl;
        // }
    }

    extractedData.shrink_to_fit();
    Print() << "Number of boxes: " << to_string(extractedData.size()) << endl;
    return extractedData;
}



void writeBinNewFormat (vector<vector<float>> data, string outFilename) {


    fstream fout;
    fout.open(outFilename, ios::out | ios::binary);

    data.shrink_to_fit();

    for (int n=0; n < data.size(); n++) {
        vector<float> box = data[n];
        for (int i=0; i < box.size(); i++) {
            fout.write(reinterpret_cast<char*>(&box[i]), sizeof(float));
        }
    }

    fout.close();

}



// write data from a whole volume to a .raw format for use in training model
void writeBin2 (vector<vector<vector<float>>> data, string outFilename) {


    fstream fout;
    fout.open(outFilename, ios::out | ios::binary);

    data.shrink_to_fit();

    int xDim = data.size();
    int yDim = data[0].size();
    int zDim = data[0][0].size();

    for (int k=0; k < zDim; k++) {
        for (int j=0; j < yDim; j++) {
            for (int i=0; i < xDim; i++) {
                fout.write(reinterpret_cast<char*>(&data[i][j][k]), sizeof(float));
            }
        }
    }

    fout.close();

}


int main (int argc, char* argv[]) {

    Initialize(argc, argv);

    int minFile;
    int maxFile;
    int lev;
    ParmParse pp;

    // range of time steps to process
    pp.query("minFile", minFile);
    pp.query("maxFile", maxFile);

    pp.query("lev", lev);

    for (int i=minFile; i <= maxFile; i++) {

        string path;
        pp.query("path", path);
        // path = "......../raw/"

        if (i < 100) {
            path += "plt0";
        } else {
            path += "plt";
        }

        path += to_string(i);
        path += "00";

        ifstream x;

        // open header
        string header = path + "/Header";
        x.open(header.c_str(), ios::in);
        if (!x.is_open()) {
            Abort("Failed to open header file :" + path);
        }

        // read in first line of header
        string str;
        x >> str;

        // read in number of components from header
        int nComp;
        x >> nComp;

        // read in variable names from header
        for (int n=0; n<nComp; n++) {
            x >> str;
        }

        // read in dimensionality from header
        int dim;
        x >> dim;

        if (dim != AMREX_SPACEDIM) {
            Print() << "\nError: you are using a " << AMREX_SPACEDIM << "D build to open a " << dim << "D plotfile\n\n";
            Abort();
        }

        // desired component index
        int component = 6;

        // level to extract the data from
        string levX = "/Level_"+to_string(lev)+"/Cell";

        string lev_file = path;
        lev_file += levX;

        // vector<vector<vector<float>>> volume = collectData(lev_file, lev, component);
        // string outFile = "../../wholeVolumes-6-1/" + to_string(i) + "-whole-6-1.raw";
        // writeBin2(volume, outFile);

        vector<vector<float>> data = collectDataNewFormat(lev_file, lev, component);
        // Print() << data.size() << endl;
        // Print() << data[0].size() << endl;
        string outFile = "../../wholeVolumesNewFormat-" + to_string(component) + "-" + to_string(lev)
                         + "/" + to_string(i) + "-wholeNewFormat-" + to_string(component) + "-" + to_string(lev) +".raw";
        writeBinNewFormat(data, outFile);

    }


}
