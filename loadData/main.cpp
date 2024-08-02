#include "AMReX_MultiFab.H"
#include "AMReX_Print.H"
#include <fstream>
#include <iostream>

#include <AMReX.H>
#include <AMReX_ParmParse.H>
#include <AMReX_VisMF.H>

using namespace std;
using namespace amrex;

/* Exctract a data from a plotfile for a given parameter at a given level.
 *
 * Run in terminal by navigating to ~/combustiondata/loadData/build/Desktop_arm_darwin_generic_mach_o_64bit-Debug/
 * and using ./loadData ../../../plt07400/
 */

int main(int argc, char* argv[]) {

    Initialize(argc, argv);

    // plotfile name
    string iFile;

    // read in parameters from inputs file
    ParmParse pp;

    // read in plotfile name
    pp.query("infile", iFile);
    if (iFile.empty())
        Abort("You must specify 'infile'");

    int comp_in_line = 0;
    pp.query("comp_in_line", comp_in_line);

    // for the Header
    string Header = iFile;
    Header += "/Header";

    // open header
    ifstream x;
    x.open(Header.c_str(), ios::in);
    if (!x.is_open()) {
        Abort("Failed to open header file: " + Header);
    }

    // read in the first line of the header
    string str;
    x >> str;

    // read in number of components from header
    int ncomp;
    x >> ncomp;

    // read in variable names from header
    for (int n=0; n<ncomp; n++) {
        x >> str;
    }

    // read in dimensionality from header
    int dim;
    x >> dim;

    if (dim != AMREX_SPACEDIM) {
        Print() << "\nError: you are using a " << AMREX_SPACEDIM << "D build to open a " << dim << "D plotfile\n\n";
        Abort();
    }

    // choose the level to extract data from
    int lev = 1;

    // storage for the Multifab
    MultiFab mf;

    string iFile_lev = iFile;

    string levX = "/Level_"+to_string(lev)+"/Cell";
    string levXX = "/Level_0"+to_string(lev)+"/Cell";

    // now read in plotfile data
    // check to see whether the user pointed to the plotfile base directory
    // or the data itself
    if (FileExists(iFile+levX+"_H")) {
        iFile_lev += levX;
    } else if (FileExists(iFile+levXX+"_H")) {
        iFile_lev += levXX;
    } else {
        Abort("No data for the specified level.");
    }

    // read in plotfile to MultiFab
    VisMF::Read(mf, iFile_lev);

    ncomp = mf.nComp();
    Print() << "Number of components in the plotfile = " << ncomp << endl;
    Print() << "Nodality of plotfile = " << mf.ixType().toIntVect() << endl;

    // get boxArray to compute number of grid points at that level
    BoxArray ba = mf.boxArray();
    Print() << "Number of grid points at level " << lev << " = " << ba.numPts() << endl;
    Print() << "Number of boxes at level " << lev << " = " << ba.size() << endl;

    // desired component index
    int component = 6;

    // given box, access data
    int boxNum = 1;
    Box mfdata = ba[boxNum];
    Print() << mfdata.size() << endl;

    Array4<Real> mfdata1 = mf.array(boxNum);
    Print() << mfdata1(505,0,48,component);

    // given location, access data
    int xloc = 505;
    int yloc = 0;
    int zloc = 48;

    for (MFIter mfi(mf,false); mfi.isValid(); ++mfi) {

        const Box& box = mfi.validbox();
        const Array4<Real>& mfdata = mf.array(mfi);

        bool found = false;

        while(found == false) {
            try {
                Print() << mfdata(xloc, yloc, zloc, component) << endl;
                found = true;
            } catch(...) {

            }
        }

    }

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
