#include "AMReX_MultiFab.H"
#include "AMReX_Print.H"
#include <fstream>
#include <iostream>
#include <vector>

#include <AMReX.H>
#include <AMReX_ParmParse.H>
#include <AMReX_VisMF.H>

using namespace std;
using namespace amrex;

int main(int argc, char* argv[]) {

    Initialize(argc, argv);

    vector<string> labels = {"A", "C", "B"};

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

    vector<string> files = {aFile, cFile, bFile};

    // set up storage variables/tools for info from input files
    string aHeader = aFile;
    string cHeader = cFile;
    string bHeader = bFile;
    vector<string> headers = {aHeader, cHeader, bHeader};

    string aStr;
    string cStr;
    string bStr;
    vector<string> strings = {aStr, cStr, bStr};

    int aNComp = 0;
    int cNComp = 0;
    int bNComp = 0;
    vector<int> nComps = {aNComp, cNComp, bNComp};

    int aDim = 0;
    int cDim = 0;
    int bDim = 0;
    vector<int> dims = {aDim, cDim, bDim};

    // level to extract the data from
    int lev = 0;
    string levX = "/Level_"+to_string(lev)+"/Cell";

    string aFile_lev;
    string cFile_lev;
    string bFile_lev;
    vector<string> lev_files = {aFile_lev, cFile_lev, bFile_lev};

    // desired component index
    int component = 6;


    // Process input files
    for (int i=0; i < files.size(); i++) {

        ifstream x;

        headers[i] += "/Header";

        // open header
        x.open(headers[i].c_str(), ios::in);
        if (!x.is_open()) {
            Abort("Failed to open header file: " + headers[i]);
        }

        // read in the first line of the header
        x >> strings[i];

        // read in the number of components from the header
        x >> nComps[i];

        // read in variable names from header
        for (int n=0; n<nComps[i]; n++) {
            x >> strings[i];
        }

        // read in dimensionality from header
        x >> dims[i];

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
    // figure out how to use pointers to make this vector!
    MultiFab aMF;
    MultiFab cMF;
    MultiFab bMF;
    vector<MultiFab> mfs;

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


    // Storage structure for the extracted data
    vector<vector<vector<vector<vector<Real>>>>> extractedData(3, vector<vector<vector<vector<Real>>>>
                                                              (4608, vector<vector<vector<Real>>>
                                                              (8, vector<vector<Real>>
                                                              (8, vector<Real>(8)))));

    // Refine 16x16x16 boxes to 8x8x8 boxes
    int boxNum = 0;

    for (int n = 0; n < mfs.size(); n++) {

        for (MFIter mfi(mfs[n], false); mfi.isValid(); ++mfi) {

            const Box& box = mfi.validbox();
            const Array4<Real>& mfdata = mfs[n].array(mfi);
            const auto lo = lbound(box);
            const auto hi = ubound(box);

            // Octant 1: xyz
            for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

                for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

                    for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

                        extractedData[n][boxNum * 8][i-lo.x][j-lo.y][k-lo.z] = mfdata(i,j,k,component);

                    }

                }

            }

            // Octant 2: Xyz
            for (int i = lo.x + box.size()[0]/2; i < hi.x; ++i) {

                for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

                    for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

                        extractedData[n][(boxNum * 8) + 1][i-(lo.x + box.size()[0]/2)][j-lo.y][k-lo.z] = mfdata(i,j,k,component);

                    }

                }

            }

            // Octant 3: XYz
            for (int i = lo.x + box.size()[0]/2; i < hi.x; ++i) {

                for (int j = lo.y + box.size()[1]/2; j < hi.y; ++j) {

                    for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

                        extractedData[n][(boxNum * 8) + 2][i-(lo.x + box.size()[0]/2)][j-(lo.y + box.size()[1]/2)][k-lo.z] = mfdata(i,j,k,component);

                    }

                }

            }

            // Octant 4: xYz
            for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

                for (int j = lo.y + box.size()[1]/2; j < hi.y; ++j) {

                    for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

                        extractedData[n][(boxNum * 8) + 3][i-lo.x][j-(lo.y + box.size()[1]/2)][k-lo.z] = mfdata(i,j,k,component);

                    }

                }

            }

            // Octant 5: xyZ
            for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

                for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

                    for (int k = lo.z + box.size()[2]/2; k < lo.z; ++k) {

                        extractedData[n][(boxNum * 8) + 4][i-lo.x][j-lo.y][k-(lo.z + box.size()[2]/2)] = mfdata(i,j,k,component);

                    }

                }

            }

            // Octant 6: XyZ
            for (int i = lo.x + box.size()[0]/2; i < hi.x; ++i) {

                for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

                    for (int k = lo.z + box.size()[2]/2; k < lo.z; ++k) {

                        extractedData[n][(boxNum * 8) + 5][i-(lo.x + box.size()[0]/2)][j-lo.y][k-(lo.z + box.size()[2]/2)] = mfdata(i,j,k,component);

                    }

                }

            }

            // Octant 7: XYZ
            for (int i = lo.x + box.size()[0]/2; i < hi.x; ++i) {

                for (int j = lo.y + box.size()[1]/2; j < hi.y; ++j) {

                    for (int k = lo.z + box.size()[2]/2; k < lo.z; ++k) {

                        extractedData[n][(boxNum * 8) + 6][i-(lo.x + box.size()[0]/2)][j-(lo.y + box.size()[1]/2)][k-(lo.z + box.size()[2]/2)] = mfdata(i,j,k,component);

                    }

                }

            }

            // Octant 8: xYZ
            for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

                for (int j = lo.y + box.size()[1]/2; j < hi.y; ++j) {

                    for (int k = lo.z + box.size()[2]/2; k < lo.z; ++k) {

                        extractedData[n][(boxNum * 8) + 7][i-lo.x][j-(lo.y + box.size()[1]/2)][k-(lo.z + box.size()[2]/2)] = mfdata(i,j,k,component);

                    }

                }

            }

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
