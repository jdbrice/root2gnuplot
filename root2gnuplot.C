//usr/bin/env root -l -b -q "$0( \"$1\", \"$2\", \"$3\" )"; exit $?
#include "TFile.h"
#include "TKey.h"
#include "TString.h"

#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdio>
#include <iomanip> // c++11
#include <cstdlib>
using namespace std;


const int col_width = 15;
const int col_precision = 5;
const std::string format_default_TH1 = "x y xlow xhigh ylow yhigh";
const std::string format_default_TH2 = "x y z";
const std::string format_default_TGraphErrors = "x y xlow xhigh ylow yhigh";


class DataFileMaker {
    public:

        struct PackedValues{
            double x, y, z;
            double xl, yl, zl;
            double xh, yh, zh;

            double dx(){
                return xh - x;
            }
            double dy(){
                return yh - y;
            }
            double dz(){
                return zh - z;
            }

        };

        DataFileMaker() {}
        ~DataFileMaker() {}

        std::vector<std::string> tmpfile_list; 
        size_t tmpfile_count_max = 0;
        size_t tmpfile_count = 0;

        void print_format( std::ofstream &tmp, string format ) {
            TString tsfmt = TString(format.c_str());
            TString token;
            Ssiz_t from = 0;
            tmp << "#";
            while ( tsfmt.Tokenize( token, from, " " ) ){
                tmp << std::left << std::setw( col_width ) ;
                tmp << token;
            }
            tmp << std::endl;
        }


        void format_line( std::ofstream &tmp, PackedValues &pv, string &format ){
            // NOW parse the format string and push data out in that order
            stringstream sstr( format );
            string token = "";
            string sep = "";
            while( std::getline(sstr, token, ' ') ){

                tmp << " " << sep << std::left << std::setw( col_width - 2 );
                if ( "x" == token )
                    tmp << pv.x;
                if ( "xlow" == token )
                    tmp << pv.xl;
                if ( "xhigh" == token )
                    tmp << pv.xh;
                if ( "y" == token )
                    tmp << pv.y;
                if ( "ylow" == token )
                    tmp << pv.yl;
                if ( "yhigh" == token )
                    tmp << pv.yh;
                if ( "z" == token )
                    tmp << pv.z;
                if ( "zlow" == token )
                    tmp << pv.zl;
                if ( "zhigh" == token )
                    tmp << pv.zh;
                if ( "dx" == token )
                    tmp << pv.dx();
                if ( "dy" == token )
                    tmp << pv.dy();
                if ( "dz" == token )
                    tmp << pv.dz();

                sep = " ";
            }
        }

        // master convert for all histogram obects (TH1, TH2, TH3)
        string convert( TH1 * h, string format, string fn, string meta = "") {

            std::ofstream tmp;
            
            tmp.open (fn, std::ofstream::out );
            


            TH3 * h3 = dynamic_cast<TH3*>( h );
            TH2 * h2 = dynamic_cast<TH2*>( h );


            if ( nullptr != h3 && h3->GetZaxis()->GetNbins() > 1 ){
                tmp << "#converted from TH3 " << meta << endl;
                convert_TH3( tmp, h3, format );
            } else if ( nullptr != h2 && h2->GetYaxis()->GetNbins() > 1 ){
                tmp << "#converted from TH2 " << meta << endl;
                convert_TH2( tmp, h2, format );
            } else {
                tmp << "#converted from TH1 " << meta << endl;
                convert_TH1( tmp, h, format );
            }

            tmp.flush();
            tmp.close();
            return fn; // return the filename
        }

        // master convert for all TGraph obects
        string convert( TGraph * g, string format, string fn, string meta = "" ) {
            std::ofstream tmp;
            tmp.open (fn, std::ofstream::out );
            
            tmp << "#converted from TGraph " << meta << endl;
            convert_TGraphErrors( tmp, g, format );

            tmp.flush();
            tmp.close();
            return fn;
        }


        void convert_TH1( std::ofstream &tmp, TH1*h, string format ){
            cout << "Converting TH1" << endl;
            if ( "" == format ){
                format = format_default_TH1;//"x y xlow xhigh ylow yhigh";
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            // Print the format to the file as column headers
            print_format( tmp, format );

            PackedValues pv;
            for ( int i = 1; i < h->GetNbinsX() + 1; i++ ){
                double vx = h->GetBinCenter( i );
                double vy = h->GetBinContent( i );
                double vey = h->GetBinError( i );

                pv.x = h->GetBinCenter( i );
                pv.xl = h->GetBinLowEdge(i);
                pv.xh = h->GetBinLowEdge(i) + h->GetBinWidth(i);
                pv.y = h->GetBinContent( i );
                pv.yl = pv.y - h->GetBinError( i );
                pv.yh = pv.y + h->GetBinError( i );

                format_line( tmp, pv, format );

                tmp << std::endl;
            }

        } // convert_TH1

        void convert_TH2( std::ofstream &tmp, TH2*h, string format ){
            cout << "Converting TH2" << endl;

            if ( "" == format ){
                format = format_default_TH2;
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            // Print the format to the file as column headers
            print_format( tmp, format ) ;

            PackedValues pv;
            for ( int ix = 1; ix < h->GetNbinsX(); ix++ ){
                for ( int iy = 1; iy < h->GetNbinsY(); iy++ ){

                    // pack the TH2 data into struct and then print formatted
                    pv.x = h->GetXaxis()->GetBinCenter( ix );
                    pv.y = h->GetYaxis()->GetBinCenter( iy );
                    pv.z = h->GetBinContent( ix, iy );
                    pv.zh = pv.z + h->GetBinError( ix, iy );
                    pv.zl = pv.z - h->GetBinError( ix, iy );
                    pv.xl = h->GetXaxis()->GetBinLowEdge(ix);
                    pv.xh = pv.xl + h->GetXaxis()->GetBinWidth(ix);
                    pv.yl = h->GetYaxis()->GetBinLowEdge(iy);
                    pv.yh = pv.yl + h->GetYaxis()->GetBinWidth(iy);

                    format_line( tmp, pv, format );
                    tmp << std::endl;
                } // loop on iy
                // double blank line per gnuplot data format
                tmp << std::endl;
            } // loop on ix
        } // convert_TH2

        void convert_TH3( std::ofstream &tmp, TH3*h, string format ){
            //TODO
            cout << "TH3 is not implemented yet!" << endl;
        }


        void convert_TGraphErrors( std::ofstream &tmp, TGraph*gr, string format ){
            cout << "Converting a TGraphErrors" << endl;

            if ( "" == format ){
                format = format_default_TGraphErrors;
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            // Print the format to the file as column headers
            print_format( tmp, format );

            PackedValues pv;
            for ( int i = 0; i < gr->GetN(); i++ ){
                double vx = gr->GetX()[i];
                double vy = gr->GetY()[i];

                pv.x = gr->GetX()[i];
                pv.xl = pv.x - gr->GetErrorXlow( i );
                pv.xh = pv.x + gr->GetErrorXhigh( i );
                pv.y = gr->GetY()[i];
                pv.yl = pv.y - gr->GetErrorYlow( i );
                pv.yh = pv.y + gr->GetErrorYhigh( i );

                // print the formatted line
                format_line( tmp, pv, format );
            }

        } // convert_TGraph

};



size_t verbosity = 1;


void root2gnuplot( TString rootfile_hist = "", TString output = "out.dat", TString format = "" ){

    cout << "Usage:" << endl;
    cout << "\troot2gnuplot.C <--help> input.root<:name> output.dat <format>, if <:name> omitted, convert all" << endl;
    cout << "\t--help print more information" << endl;

    if ( rootfile_hist == "--help" ){
        cout << "help" << endl << endl;
        cout << "format specifiers (case sensitive):" << endl;
        cout << "\tx, y, z: coordinate from first, second, third axis for TH1, TH2, TH3 histograms" << endl << endl;
        cout << "\t<axis>low: value on <axis> at lower error bar, example: xlow, ylow, zlow" << endl << endl;
        cout << "\t<axis>high: value on <axis> at upper error bar, example: xhigh, yhigh, zhigh" << endl << endl;
        cout << "\td<axis>: value on <axis> at upper error bar minus value on <axis>, example: dx, dy, dz" << endl << endl << endl;

        cout << "default formats:" << endl;
        cout << "\tTH1: " << format_default_TH1 << endl;
        return;
    }


    bool convert_all = rootfile_hist.First( ':' ) == -1;
    int index_split = rootfile_hist.First( ':' );
    if ( index_split < 0 )
        index_split = rootfile_hist.Length();

    TString rfn = rootfile_hist(0, index_split);
    TString hfn = rootfile_hist(index_split+1, rootfile_hist.Length());

    if ( verbosity > 0 ) {
        cout << "ROOT Input File: " << rfn << endl;
        if ( hfn != "" )
            cout << "ROOT Input Object: " << hfn << endl;
        else 
            cout << "Converting all objects in file" << endl;
        cout << "Ouput File: " << output << endl;
    }

    TFile *rf = new TFile( rfn.Data() );
    if ( !rf || rf->IsOpen() == false ){
        cerr << "Cannot open input file" << endl;
        return;
    }

    TObject * obj = rf->Get( hfn.Data() );
    if ( !obj &&  hfn != "" ) {
        cerr << "Cannot get object named: " << hfn << endl;
        return;
    }

    DataFileMaker dfm;
    string metadata = "";
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    stringstream dtss;
    dtss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

    if ( obj ){
        metadata = "name: " + string(hfn.Data()) + ", file: " + string(rfn.Data()) + ", date: " + dtss.str();
        if ( dynamic_cast<TH1*>( obj ) ){
            cout << "Converting '" << hfn << "' as a histogram" << endl; 
            dfm.convert( dynamic_cast<TH1*>( obj ), format.Data(), output.Data(), metadata );
        } else if ( dynamic_cast<TGraph*>( obj ) ){
            cout << "Converting '" << hfn << "' as a graph" << endl; 
            dfm.convert( dynamic_cast<TGraph*>( obj ), format.Data(), output.Data(), metadata );
        }
        return;
    }

    // convert all objects in file
    if ( !obj && convert_all ) {
        TIter next(rf->GetListOfKeys());
        TKey *key;
        while ( (key = (TKey*)next()) ) {
            TString loutput = TString::Format( "%s_%s.dat", output.Data(), key->GetName() );
            cout << "Converting " << key->GetName() << "[" << key->GetClassName() << "] into " << loutput << endl;
            metadata = "name: " + string(key->GetName()) + ", file: " + string(rfn.Data()) + ", date: " + dtss.str();
            obj = rf->Get( key->GetName() );
            TGraph * g = dynamic_cast<TGraph*>( obj );
            TH1 * h = dynamic_cast<TH1*>( obj );

            if ( g != nullptr )
                dfm.convert( g, format.Data(), loutput.Data(), metadata );
            else if ( h != nullptr ){
                dfm.convert( h, format.Data(), loutput.Data(), metadata );
            }
        }
    }
}
