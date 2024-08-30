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
vector<vector<vector<float>>> collectData (string lev_file, int component) {

    MultiFab mf;

    // read data into multifab
    VisMF::Read(mf, lev_file);

    // int nComp = mf.nComp();
    // Print() << "Number of components in file " << filename << " = " << nComp << endl;
    // Print() << "Nodality of file " << filename << " = " << mf.ixType().toIntVect() << endl;

    // // get boxArray to compute number of grid points at that level
    // BoxArray ba = mf.boxArray();
    // Print() << "Number of grid points at level " << lev << " in file " << filename << " = " << ba.numPts() << endl;
    // Print() << "Number of boxes at level " << lev << " in file " << filename << " = " << ba.size() << endl;
    // Print() << endl;

    // storage vector for extracted data
    vector<vector<vector<float>>> extractedData(256, vector<vector<float>>(96, vector<float>(96)));

    for (MFIter mfi(mf, false); mfi.isValid(); ++mfi) {

        const Box& box = mfi.validbox();
        const Array4<Real>& mfdata = mf.array(mfi);
        const auto lo = lbound(box);
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


// write a data to a .raw format for use in training model
void writeBin (vector<vector<vector<float>>> data, string outFilename) {


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
    ParmParse pp;

    pp.query("minFile", minFile);
    pp.query("maxFile", maxFile);

    for (int i=minFile; i <= maxFile; i++) {

        string path;
        pp.query("path", path);
        // path = "....../raw/"

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
        int component = 5;

        // level to extract the data from
        int lev = 0;
        string levX = "/Level_"+to_string(lev)+"/Cell";

        string lev_file = path;
        lev_file += levX;

        vector<vector<vector<float>>> volume = collectData(lev_file, component);

        string outFile = "../../volumes/" + to_string(i) + ".raw";
        writeBin(volume, outFile);

    }

}
