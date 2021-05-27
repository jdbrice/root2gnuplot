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
#include <cstdlib>
using namespace std;


const int col_width = 15;
const int col_precision = 5;

class DataFileMaker {
    public:

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

        // master convert (handles all histogram obects)
        string convert( TH1 * h, string format, string fn, bool binary = false ) {

            std::ofstream tmp;
            
            tmp.open (fn, std::ofstream::out );
            


            TH3 * h3 = dynamic_cast<TH3*>( h );
            TH2 * h2 = dynamic_cast<TH2*>( h );


            if ( nullptr != h3 && h3->GetZaxis()->GetNbins() > 1 ){
                tmp << "#converted from TH3" << endl;
                convert_TH3( tmp, h3, format );
            } else if ( nullptr != h2 && h2->GetYaxis()->GetNbins() > 1 ){
                tmp << "#converted from TH2" << endl;
                convert_TH2( tmp, h2, format );
            } else {
                tmp << "#converted from TH1" << endl;
                convert_TH1( tmp, h, format );
            }

            tmp.flush();
            tmp.close();
            return fn; // return the filename
        }

        // master convert (handles all TGraph obects)
        string convert( TGraph * g, string format, string fn, bool binary = false ) {
            std::ofstream tmp;
            tmp.open (fn, std::ofstream::out );
            
            tmp << "#converted from TGraph" << endl;
            convert_TGraphErrors( tmp, g, format );

            tmp.flush();
            tmp.close();
            return fn;
        }


        void convert_TH1( std::ofstream &tmp, TH1*h, string format, bool binary = false ){
            cout << "Converting TH1" << endl;
            if ( "" == format ){
                format = "x y xlow xhigh ylow yhigh";
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            print_format( tmp, format );

            vector<double> x, y, x_low, x_high, y_low, y_high;
            for ( int i = 1; i < h->GetNbinsX() + 1; i++ ){
                double vx = h->GetBinCenter( i );
                double vy = h->GetBinContent( i );
                double vey = h->GetBinError( i );

                x.push_back( vx );
                x_low.push_back( h->GetBinLowEdge(i) );
                x_high.push_back( h->GetBinLowEdge(i) + h->GetBinWidth(i) );
                y.push_back( vy );
                y_low.push_back( vy - vey );
                y_high.push_back( vy + vey );
            }

            assert( y.size() == x.size() );
            assert( x_low.size() == x.size() );
            assert( x_high.size() == x.size() );
            assert( y_low.size() == x.size() );
            assert( y_high.size() == x.size() );


            for (unsigned int i = 0; i < x.size(); i++){

                // NOW parse the format string and push data out in that order
                stringstream sstr( format );
                string token = "";
                string sep = "";
                while( std::getline(sstr, token, ' ') ){


                    tmp << " " << sep << std::left << std::setw( col_width - 2 );
                    if ( "x" == token )
                        tmp << x[i];
                    if ( "xlow" == token )
                        tmp << x_low[i];
                    if ( "xhigh" == token )
                        tmp << x_high[i];
                    if ( "y" == token )
                        tmp << y[i];
                    if ( "ylow" == token )
                        tmp << y_low[i];
                    if ( "yhigh" == token )
                        tmp << y_high[i];
                    if ( "dx" == token )
                        tmp << (x_high[i] - x[i]);
                    if ( "dy" == token )
                        tmp << (y_high[i] - y[i]);

                    sep = " ";
                }

                tmp << std::endl;
            }

        } // convert_TH1

        void convert_TH2( std::ofstream &tmp, TH2*h, string format, bool binary = false ){
            cout << "Converting TH2" << endl;

            if ( "" == format ){
                format = "x y z";
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            print_format( tmp, format ) ;

            
            for ( int ix = 1; ix < h->GetNbinsX(); ix++ ){
                for ( int iy = 1; iy < h->GetNbinsY(); iy++ ){
                    
                    double vx = h->GetXaxis()->GetBinCenter( ix );
                    double vy = h->GetYaxis()->GetBinCenter( iy );
                    double vz = h->GetBinContent( ix, iy );
                    double vez = h->GetBinError( ix, iy );
                    double vxlow = h->GetXaxis()->GetBinLowEdge(ix);
                    double vxhigh = vxlow + h->GetXaxis()->GetBinWidth(ix);

                    double vylow = h->GetYaxis()->GetBinLowEdge(iy);
                    double vyhigh = vylow + h->GetYaxis()->GetBinWidth(iy);

                    // NOW parse the format string and push data out in that order
                    stringstream sstr( format );
                    string token = "";
                    string sep = "";
                    while( std::getline(sstr, token, ' ') ){

                        tmp << " " << sep << std::left << std::setw( col_width - 2 );
                        if ( "x" == token )
                            tmp << vx;
                        if ( "xlow" == token )
                            tmp << vxlow;
                        if ( "xhigh" == token )
                            tmp << vxhigh;
                        if ( "y" == token )
                            tmp << vy;
                        if ( "ylow" == token )
                            tmp << vylow;
                        if ( "yhigh" == token )
                            tmp << vyhigh;
                        if ( "z" == token )
                            tmp << vz;
                        if ( "zlow" == token )
                            tmp << vz-vez;
                        if ( "zhigh" == token )
                            tmp << vz+vez;
                        if ( "dx" == token )
                            tmp << (vxhigh - vx);
                        if ( "dy" == token )
                            tmp << (vyhigh - vy);
                        if ( "dz" == token )
                            tmp << (vez);

                        sep = " ";
                    }

                    tmp << std::endl;
                }
                tmp << std::endl;
            }

            return;

        } // convert_TH2

        void convert_TH3( std::ofstream &tmp, TH3*h, string format, bool binary = false ){
        }


        void convert_TGraphErrors( std::ofstream &tmp, TGraph*gr, string format, bool binary = false ){
            cout << "Converting a TGraphErrors" << endl;

            if ( "" == format ){
                format = "x y xlow xhigh ylow yhigh";
                cout << TString::Format("No format provided, default is: %s", format.c_str() ) << endl;
            }

            print_format( tmp, format );

            vector<double> x, y, x_low, x_high, y_low, y_high;
            for ( int i = 0; i < gr->GetN(); i++ ){
                double vx = gr->GetX()[i];
                double vy = gr->GetY()[i];

                x.push_back( vx );
                x_low.push_back( vx - gr->GetErrorXlow( i ) );
                x_high.push_back( vx + gr->GetErrorXhigh( i ) );
                y.push_back( vy );
                y_low.push_back( vy - gr->GetErrorYlow( i ) );
                y_high.push_back( vy + gr->GetErrorYhigh( i ) );
            }

            assert( y.size() == x.size() );
            assert( x_low.size() == x.size() );
            assert( x_high.size() == x.size() );
            assert( y_low.size() == x.size() );
            assert( y_high.size() == x.size() );


            for (unsigned int i = 0; i < x.size(); i++){

                // NOW parse the format string and push data out in that order
                stringstream sstr( format );
                string token = "";
                string sep = "";
                while( std::getline(sstr, token, ' ') ){

                    tmp << " " << sep << std::left << std::setw( col_width - 2 );
                    if ( "x" == token )
                        tmp << x[i];
                    if ( "xlow" == token )
                        tmp << x_low[i];
                    if ( "xhigh" == token )
                        tmp << x_high[i];
                    if ( "y" == token )
                        tmp << y[i];
                    if ( "ylow" == token )
                        tmp << y_low[i];
                    if ( "yhigh" == token )
                        tmp << y_high[i];
                    if ( "dx" == token )
                        tmp << (x_high[i] - x[i]);
                    if ( "dy" == token )
                        tmp << (y_high[i] - y[i]);

                    sep = " ";
                }

                tmp << std::endl;
            }

        } // convert_TH1

};



size_t verbosity = 1;


void root2gnuplot( TString rootfile_hist = "", TString output = "out.dat", TString format = "" ){

    cout << "Usage:" << endl;
    cout << "\tr2g input.root<:name> output.dat <format>, if <:name> omitted, convert all" << endl;

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

    if ( obj ){
        if ( dynamic_cast<TH1*>( obj ) ){
            cout << "Converting '" << hfn << "' as a histogram" << endl; 
            dfm.convert( dynamic_cast<TH1*>( obj ), format.Data(), output.Data() );
        } else if ( dynamic_cast<TGraph*>( obj ) ){
            cout << "Converting '" << hfn << "' as a graph" << endl; 
            dfm.convert( dynamic_cast<TGraph*>( obj ), format.Data(), output.Data() );
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
            obj = rf->Get( key->GetName() );
            TGraph * g = dynamic_cast<TGraph*>( obj );
            TH1 * h = dynamic_cast<TH1*>( obj );

            if ( g != nullptr )
                dfm.convert( g, format.Data(), loutput.Data() );
            else if ( h != nullptr ){
                dfm.convert( h, format.Data(), loutput.Data() );
            }
        }
    }
}
