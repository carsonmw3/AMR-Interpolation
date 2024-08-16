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

/* Version 2: goes block by block, storing entire dataset as 1 volume
 * with a value at each coordinate, then allows to refine into blocks
 * of a chosen size.
 */


// Reads input data and puts it into one big vector representing the whole volume
vector<vector<vector<float>>> collectData (string lev_file, string filename, int lev, int component) {

    MultiFab mf;

    // read data into multifab
    VisMF::Read(mf, lev_file);

    int nComp = mf.nComp();
    Print() << "Number of components in file " << filename << " = " << nComp << endl;
    Print() << "Nodality of file " << filename << " = " << mf.ixType().toIntVect() << endl;

    // get boxArray to compute number of grid points at that level
    BoxArray ba = mf.boxArray();
    Print() << "Number of grid points at level " << lev << " in file " << filename << " = " << ba.numPts() << endl;
    Print() << "Number of boxes at level " << lev << " in file " << filename << " = " << ba.size() << endl;
    Print() << endl;

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
void writeBin (vector<vector<vector<vector<float>>>> aData,
              vector<vector<vector<vector<float>>>> cData,
              vector<vector<vector<vector<float>>>> bData,
              string outFilename) {


    fstream fout;
    fout.open(outFilename, ios::out | ios::binary);

    aData.shrink_to_fit();
    cData.shrink_to_fit();
    bData.shrink_to_fit();

    if (aData.size() != cData.size() || aData.size() != bData.size() || cData.size() != bData.size()) {
        Print() << "Datasets are not the same size!";
        Abort();
    }

    int numBlocks = aData.size();
    int xDim = aData[0].size();
    int yDim = aData[0][0].size();
    int zDim = aData[0][0][0].size();

    for (int n = 0; n < numBlocks; n++) {

        for (int k=0; k < zDim; k++) {
            for (int j=0; j < yDim; j++) {
                for (int i=0; i < xDim; i++) {
                    fout.write(reinterpret_cast<char*>(&aData[n][i][j][k]), sizeof(float));
                }
            }
        }

        for (int k=0; k < zDim; k++) {
            for (int j=0; j < yDim; j++) {
                for (int i=0; i < xDim; i++) {
                    fout.write(reinterpret_cast<char*>(&cData[n][i][j][k]), sizeof(float));
                }
            }
        }

        for (int k=0; k < zDim; k++) {
            for (int j=0; j < yDim; j++) {
                for (int i=0; i < xDim; i++) {
                    fout.write(reinterpret_cast<char*>(&bData[n][i][j][k]), sizeof(float));
                }
            }
        }

    }

    fout.close();

}


// Refine the volume to NxNxN blocks
vector<vector<vector<vector<float>>>> toBlocks (vector<vector<vector<float>>> data, int dimension) {

    data.shrink_to_fit();
    int xDim = data.size();
    int yDim = data[0].size();
    int zDim = data[0][0].size();

    // check if valid block size
    if (xDim % dimension != 0 || yDim % dimension != 0 || zDim % dimension != 0) {
        Print() << "Not a valid block size!" << endl;
        Abort();
    }

    // compute number of steps for each dimension
    int xSteps = xDim / dimension;
    int ySteps = yDim / dimension;
    int zSteps = zDim / dimension;

    // create output storage
    vector<vector<vector<vector<float>>>> refined;

    for (int k = 0; k < zSteps; k++) {

        for (int j = 0; j < ySteps; j++) {

            for (int i = 0; i < xSteps; i++) {

                int xLo = i * dimension;
                int yLo = j * dimension;
                int zLo = k * dimension;

                vector<vector<vector<float>>> block(dimension, vector<vector<float>>(dimension, vector<float>(dimension)));

                for (int z = 0; z < dimension; z++) {

                    for (int y = 0; y < dimension; y++) {

                        for (int x = 0; x < dimension; x++) {

                            block[x][y][z] = data[xLo + x][yLo + y][zLo + z];

                        }

                    }

                }

                refined.push_back(block);

            }

        }

    }

    refined.shrink_to_fit();
    return refined;

}




// given the index of a block, determines its coordinates in the greater volume
IntVect location (int index, vector<vector<vector<float>>> volume, vector<vector<vector<vector<float>>>> blocks, int blockDim) {

    // get dimensions of greater volume
    volume.shrink_to_fit();
    int xDim = volume.size();
    int yDim = volume[0].size();
    int zDim = volume[0][0].size();

    // compute number of blocks for each dimension, and total number of blocks
    int xBlocks = xDim / blockDim;
    int yBlocks = yDim / blockDim;
    int zBlocks = zDim / blockDim;
    int numBlocks = xBlocks * yBlocks * zBlocks;

    // calculate the lo coordinate for each dimension for the given block
    int yPer = numBlocks / zBlocks / yBlocks;
    int zPer = numBlocks / zBlocks;

    int xLo = ((index % zPer) % yPer) * blockDim;
    int yLo = ((index % zPer) / yPer) * blockDim;
    int zLo = (index / zPer) * blockDim;

    IntVect location;
    location[0] = xLo;
    location[1] = yLo;
    location[2] = zLo;

    return location;

}




int main (int argc, char* argv[]) {

    Initialize(argc, argv);

    vector<string> labels = {"A", "C", "B"};

    // read in input files
    string aFile, cFile, bFile;
    ParmParse pp;

    pp.query("a", aFile);
    pp.query("c", cFile);
    pp.query("b", bFile);

    int comp_in_line = 0;
    pp.query("comp_in_line", comp_in_line);

    vector<string> files = {aFile, cFile, bFile};

    // set up storage variables/tools for info from input files
    string aHeader = aFile, cHeader = cFile, bHeader = bFile;
    vector<string> headers = {aHeader, cHeader, bHeader};

    string aStr, cStr, bStr;
    vector<string> strings = {aStr, cStr, bStr};

    int aNComp = 0, cNComp = 0, bNComp = 0;
    vector<int> nComps = {aNComp, cNComp, bNComp};

    int aDim = 0, cDim = 0, bDim = 0;
    vector<int> dims = {aDim, cDim, bDim};

    // level to extract the data from
    int lev = 0;
    string levX = "/Level_"+to_string(lev)+"/Cell";

    string aFile_lev, cFile_lev, bFile_lev;
    vector<string> lev_files = {aFile_lev, cFile_lev, bFile_lev};

    // desired component index
    int component = 5;

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


    vector<vector<vector<float>>> aVolume = collectData(lev_files[0], labels[0], lev, component);
    vector<vector<vector<float>>> cVolume = collectData(lev_files[1], labels[1], lev, component);
    vector<vector<vector<float>>> bVolume = collectData(lev_files[2], labels[2], lev, component);
    // writeBin(aVolume, "toVisualize.raw");

    vector<vector<vector<vector<float>>>> aBlocks = toBlocks(aVolume, 8);
    vector<vector<vector<vector<float>>>> cBlocks = toBlocks(cVolume, 8);
    vector<vector<vector<vector<float>>>> bBlocks = toBlocks(bVolume, 8);

    // IntVect loc = location(2464, aVolume, aBlocks, 8);
    // Print() << loc << endl;
    // writeBin(aBlocks[2464], "Block.raw");

    writeBin(aBlocks, cBlocks, bBlocks, "../../../data.raw");

    // vector<float> test(1000);
    // fstream fout;
    // fout.open("../../../data.raw", ios::in | ios::binary);
    // for (int i = 0; i < 1000; i++) {
    //     fout.read(reinterpret_cast<char*>(&test[i]), sizeof(float));
    // }
    // fout.close();
    // for (int i = 0; i < 1000; i++) {
    //     Print() << test[i] << endl;
    // }

    Print() << aBlocks[2389][0][2][4] << endl;

}




/* Version 1: goes block by block, storing new 8x8x8 blocks without
 * position information.
 */

// int main(int argc, char* argv[]) {

//     Initialize(argc, argv);

//     vector<string> labels = {"A", "C", "B"};

//     // read in input files
//     string aFile, cFile, bFile;
//     ParmParse pp;

//     pp.query("a", aFile);

//     pp.query("c", cFile);
//     pp.query("b", bFile);

//     int comp_in_line = 0;
//     pp.query("comp_in_line", comp_in_line);

//     vector<string> files = {aFile, cFile, bFile};

//     // set up storage variables/tools for info from input files
//     string aHeader = aFile, cHeader = cFile, bHeader = bFile;
//     vector<string> headers = {aHeader, cHeader, bHeader};

//     string aStr, cStr, bStr;
//     vector<string> strings = {aStr, cStr, bStr};

//     int aNComp = 0, cNComp = 0, bNComp = 0;
//     vector<int> nComps = {aNComp, cNComp, bNComp};

//     int aDim = 0, cDim = 0, bDim = 0;
//     vector<int> dims = {aDim, cDim, bDim};

//     // level to extract the data from
//     int lev = 0;
//     string levX = "/Level_"+to_string(lev)+"/Cell";

//     string aFile_lev, cFile_lev, bFile_lev;
//     vector<string> lev_files = {aFile_lev, cFile_lev, bFile_lev};

//     // desired component index
//     int component = 6;


//     // Process input files
//     for (int i=0; i < files.size(); i++) {

//         ifstream x;

//         headers[i] += "/Header";

//         // open header
//         x.open(headers[i].c_str(), ios::in);
//         if (!x.is_open()) {
//             Abort("Failed to open header file: " + headers[i]);
//         }

//         // read in the first line of the header
//         x >> strings[i];

//         // read in the number of components from the header
//         x >> nComps[i];

//         // read in variable names from header
//         for (int n=0; n<nComps[i]; n++) {
//             x >> strings[i];
//         }

//         // read in dimensionality from header
//         x >> dims[i];

//         if (dims[i] != AMREX_SPACEDIM) {
//             Print() << "\nError: you are using a " << AMREX_SPACEDIM << "D build to open a " << dims[i] << "D plotfile\n\n";
//             Abort();
//         }

//         lev_files[i] = files[i];

//         // check to see whether user pointed to plotfile base directory
//         if (FileExists(files[i]+levX+"_H")) {
//             lev_files[i] += levX;
//         } else {
//             Abort("No data for the specified level.");
//         }


//     }


//     // create array of multifabs
//     Array<MultiFab, 3> mfs;


//     // read data into multifabs
//     for (int i = 0; i < mfs.size(); i++) {
//         VisMF::Read(mfs[i], lev_files[i]);

//         // nComps[i] = mfs[i].nComp();
//         // Print() << "Number of components in file " << labels[i] << " = " << nComps[i] << endl;
//         // Print() << "Nodality of file " << labels[i] << " = " << mfs[i].ixType().toIntVect() << endl;

//         // // get boxArray to compute number of grid points at that level
//         // BoxArray ba = mfs[i].boxArray();
//         // Print() << "Number of grid points at level " << lev << " in file " << labels[i] << " = " << ba.numPts() << endl;
//         // Print() << "Number of boxes at level " << lev << " in file " << labels[i] << " = " << ba.size() << endl;
//         // Print() << endl;

//     }


//     // Storage structure for the extracted data
//     vector<vector<vector<vector<vector<float>>>>> extractedData(3, vector<vector<vector<vector<float>>>>
//         (4608, vector<vector<vector<float>>>(8, vector<vector<float>>(8, vector<float>(8)))));

//     // Refine 16x16x16 boxes to 8x8x8 boxes

//     for (int n = 0; n < mfs.size(); n++) {

//         int boxNum = 0;

//         for (MFIter mfi(mfs[n], false); mfi.isValid(); ++mfi) {

//             const Box& box = mfi.validbox();
//             const Array4<Real>& mfdata = mfs[n].array(mfi);
//             const auto lo = lbound(box);
//             const auto hi = ubound(box);
//             Print() << "Box " << boxNum << ": " << lo << " | " << hi << endl;

//             // Octant 1: xyz
//             for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

//                 for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

//                     for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

//                         extractedData[n][boxNum * 8][i-lo.x][j-lo.y][k-lo.z] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }

//             // Octant 2: Xyz
//             for (int i = lo.x + box.size()[0]/2; i <= hi.x; ++i) {

//                 for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

//                     for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

//                         extractedData[n][(boxNum * 8) + 1][i-(lo.x + box.size()[0]/2)][j-lo.y][k-lo.z] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }

//             // Octant 3: XYz
//             for (int i = lo.x + box.size()[0]/2; i <= hi.x; ++i) {

//                 for (int j = lo.y + box.size()[1]/2; j <= hi.y; ++j) {

//                     for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

//                         extractedData[n][(boxNum * 8) + 2][i-(lo.x + box.size()[0]/2)][j-(lo.y + box.size()[1]/2)][k-lo.z] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }

//             // Octant 4: xYz
//             for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

//                 for (int j = lo.y + box.size()[1]/2; j <= hi.y; ++j) {

//                     for (int k = lo.z; k < lo.z + box.size()[2]/2; ++k) {

//                         extractedData[n][(boxNum * 8) + 3][i-lo.x][j-(lo.y + box.size()[1]/2)][k-lo.z] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }

//             // Octant 5: xyZ
//             for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

//                 for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

//                     for (int k = lo.z + box.size()[2]/2; k <= hi.z; ++k) {

//                         extractedData[n][(boxNum * 8) + 4][i-lo.x][j-lo.y][k-(lo.z + box.size()[2]/2)] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }

//             // Octant 6: XyZ
//             for (int i = lo.x + box.size()[0]/2; i <= hi.x; ++i) {

//                 for (int j = lo.y; j < lo.y + box.size()[1]/2; ++j) {

//                     for (int k = lo.z + box.size()[2]/2; k <= hi.z; ++k) {

//                         extractedData[n][(boxNum * 8) + 5][i-(lo.x + box.size()[0]/2)][j-lo.y][k-(lo.z + box.size()[2]/2)] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }

//             // Octant 7: XYZ
//             for (int i = lo.x + box.size()[0]/2; i <= hi.x; ++i) {

//                 for (int j = lo.y + box.size()[1]/2; j <= hi.y; ++j) {

//                     for (int k = lo.z + box.size()[2]/2; k <= hi.z; ++k) {

//                         extractedData[n][(boxNum * 8) + 6][i-(lo.x + box.size()[0]/2)][j-(lo.y + box.size()[1]/2)][k-(lo.z + box.size()[2]/2)] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }

//             // Octant 8: xYZ
//             for (int i = lo.x; i < lo.x + box.size()[0]/2; ++i) {

//                 for (int j = lo.y + box.size()[1]/2; j <= hi.y; ++j) {

//                     for (int k = lo.z + box.size()[2]/2; k <= hi.z; ++k) {

//                         extractedData[n][(boxNum * 8) + 7][i-lo.x][j-(lo.y + box.size()[1]/2)][k-(lo.z + box.size()[2]/2)] = static_cast<float>(mfdata(i,j,k,component));

//                     }

//                 }

//             }
//             boxNum++;
//         }

//     }



//     // write a box to raw binary file

//     vector<vector<vector<float>>> toVisualize = extractedData[0][980];

//     fstream fout;

//     fout.open("toVisualize.raw", ios::out | ios::binary);

//     for (int i=0; i < 8; i++) {
//         for (int j=0; j < 8; j++) {
//             for (int k=0; k < 8; k++) {
//                 fout.write(reinterpret_cast<char*>(&toVisualize[i][j][k]), sizeof(float));
//             }
//         }
//     }
//     fout.close();



//     // // test if raw file is accurate
//     // vector<vector<vector<float>>> toVisualize2 = toVisualize;

//     // fout.open("toVisualize.raw", ios::in | ios::binary);

//     // for (int i=0; i < 8; i++) {
//     //     for (int j=0; j < 8; j++) {
//     //         for (int k=0; k < 8; k++) {
//     //             fout.read(reinterpret_cast<char*>(&toVisualize2[i][j][k]), sizeof(float));
//     //         }
//     //     }
//     // }
//     // fout.close();

//     // for (int i=0; i < 8; i++) {
//     //     for (int j=0; j < 8; j++) {
//     //         for (int k=0; k < 8; k++) {
//     //             float a = toVisualize[i][j][k];
//     //             float b = toVisualize2[i][j][k];
//     //             Print() << to_string(a == b);
//     //         }
//     //     }
//     // }


//     // size  and location of boxes
//     // Print() << "Box " << boxNum << " size: " << mfdata.size() << endl;
//     // Print() << "Box " << boxNum << " lower corner: " << mfdata.smallEnd() << endl;
//     // Print() << "Box " << boxNum << " upper corner: " << mfdata.bigEnd() << endl;
// }




