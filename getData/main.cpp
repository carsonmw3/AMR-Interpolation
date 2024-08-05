#include "AMReX_MultiFab.H"
#include "AMReX_Print.H"
#include <fstream>
#include <iostream>
#include <list>

#include <AMReX.H>
#include <AMReX_ParmParse.H>
#include <AMReX_VisMF.H>

using namespace std;
using namespace amrex;

int main(int argc, char* argv[]) {

    Initialize(argc, argv);

    Array<string, 3> labels = {"A", "C", "B"};

    // read in input files
    string aFile;
    string cFile;
    string bFile;
    ParmParse pp;

    pp.query("a", aFile);
    pp.query("c", cFile);
    pp.query("b", bFile);

    int comp_in_line = 0;
    pp.query("comp_in_line", comp_in_line);

    Array<string,3> files = {aFile, cFile, bFile};

    // set up storage variables/tools for info from input files
    string aHeader = aFile;
    string cHeader = cFile;
    string bHeader = bFile;
    Array<string, 3> headers = {aHeader, cHeader, bHeader};

    Array<ifstream, 3> streams;

    string aStr;
    string cStr;
    string bStr;
    Array<string, 3> strings = {aStr, cStr, bStr};

    int aNComp = 0;
    int cNComp = 0;
    int bNComp = 0;
    Array<int, 3> nComps = {aNComp, cNComp, bNComp};

    int aDim = 0;
    int cDim = 0;
    int bDim = 0;
    Array<int, 3> dims = {aDim, cDim, bDim};

    // level to extract the data from
    int lev = 0;
    string levX = "/Level_"+to_string(lev)+"/Cell";

    string aFile_lev;
    string cFile_lev;
    string bFile_lev;
    Array<string, 3> lev_files = {aFile_lev, cFile_lev, bFile_lev};

    // desired component index
    int component = 6;


    // Process input files
    for (int i=0; i < files.size(); i++) {

        headers[i] += "/Header";

        // open header
        streams[i].open(headers[i].c_str(), ios::in);
        if (!streams[i].is_open()) {
            Abort("Failed to open header file: " + headers[i]);
        }

        // read in the first line of the header
        streams[i] >> strings[i];

        // read in the number of components from the header
        streams[i] >> nComps[i];

        // read in variable names from header
        for (int n=0; n<nComps[i]; n++) {
            streams[i] >> strings[i];
        }

        // read in dimensionality from header
        streams[i] >> dims[i];

        if (dims[i] != AMREX_SPACEDIM) {
            Print() << "\nError: you are using a " << AMREX_SPACEDIM << "D build to open a " << dims[i] << "D plotfile\n\n";
            Abort();
        }

        lev_files[i] = files[i];

        // check to see whether user pointed to plotfile base directory
        if (FileExists(files[i]+levX+"_H")) {
            lev_files[i] += levX;
        } else {
            Abort("No data for the specified level.");
        }


    }


    // create multifabs
    MultiFab aMF;
    MultiFab cMF;
    MultiFab bMF;
    Array<MultiFab, 3> mfs;

    // read data into multifabs
    for (int i = 0; i < mfs.size(); i++) {
        VisMF::Read(mfs[i], lev_files[i]);

        nComps[i] = mfs[i].nComp();
        Print() << "Number of components in file " << labels[i] << " = " << nComps[i] << endl;
        Print() << "Nodality of file " << labels[i] << " = " << mfs[i].ixType().toIntVect() << endl;

        // get boxArray to compute number of grid points at that level
        BoxArray ba = mfs[i].boxArray();
        Print() << "Number of grid points at level " << lev << " in file " << labels[i] << " = " << ba.numPts() << endl;
        Print() << "Number of boxes at level " << lev << " in file " << labels[i] << " = " << ba.size() << endl;
        Print() << endl;
    }


    // Extract data from the files
    for (int i=0; i < mfs.size(); i++) {

        for (MFIter mfi(mfs[i], false); mfi.isValid(); ++mfi) {



        }

    }


    // // size  and location of boxes
    // for (int boxNum = 0; boxNum < ba.size(); boxNum++) {
    //     Box mfdata = ba[boxNum];
    //     Print() << "Box " << boxNum << " size: " << mfdata.size() << endl;
    //     Print() << "Box " << boxNum << " lower corner: " << mfdata.smallEnd() << endl;
    //     Print() << "Box " << boxNum << " upper corner: " << mfdata.bigEnd() << endl;
    // }


    // Array4<Real> mfdata1 = mf.array(boxNum);
    // Print() << mfdata1(505,0,48,component);

    // // given location, access data
    // int xloc = 505;
    // int yloc = 0;
    // int zloc = 48;

    // for (MFIter mfi(mf,false); mfi.isValid(); ++mfi) {

    //     const Box& box = mfi.validbox();
    //     const Array4<Real>& mfdata = mf.array(mfi);

    //     bool found = false;

    //     while(found == false) {
    //         try {
    //             Print() << mfdata(xloc, yloc, zloc, component) << endl;
    //             found = true;
    //         } catch(...) {

    //         }
    //     }

    // }

    // for (MFIter mfi(mf,false); mfi.isValid(); ++mfi) {

    //     const Box& box = mfi.validbox();
    //     const Array4<Real>& mfdata = mf.array(mfi);

    //     ParallelFor(box, [=] AMREX_GPU_DEVICE (int i, int j, int k){
    //         Real temp = mfdata(i, j, k, 5);
    //         cout << temp;
    //     });

    // }


    // for (MFIter mfi(mf,false); mfi.isValid(); ++mfi) {

    //     const Box& bx = mfi.validbox();
    //     const auto lo = lbound(bx);
    //     const auto hi = ubound(bx);

    //     const Array4<Real>& mfdata = mf.array(mfi);

    //     if (comp_in_line == 1) {
    //         cout << mf[mfi];
    //     } else {
    //         for (auto n=0; n<ncomp; ++n) {
    //             for (auto k = lo.z; k <= hi.z; ++k) {
    //                 for (auto j = lo.y; j <= hi.y; ++j) {
    //                     for (auto i = lo.x; i <= hi.x; ++i) {
    //                         cout << i << " " << j << " " << k << " " << n << " " << mfdata(i,j,k,n) << '\n';
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // } // end MFIter

}
