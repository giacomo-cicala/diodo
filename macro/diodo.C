#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TAxis.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMath.h"
#include "TMatrixD.h"
#include <iostream>
#include <iomanip>

// Macro per analisi Diodi con fit esponenziali per Silicio e Germanio

void fit_calibrazione()
{
    const char *filename = "data/dati_calibrazione.txt";
    TGraphErrors *gr = new TGraphErrors(filename, "%lg %lg %lg %lg");

    if (gr->GetN() == 0)
    {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }

    gr->SetTitle("Calibrazione;Multimetro (mV);Oscilloscopio (mV)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);

    TF1 *f0 = new TF1("f_calib", "[0]+[1]*x", 0., 1000.);
    f0->SetLineColor(kRed);

    gr->Fit(f0, "Q");
    gr->Draw("AP");

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n>>> Calibrazione (" << filename << " ):\n";
    std::cout << "Intercetta: " << f0->GetParameter(0) << " +/- " << f0->GetParError(0) << std::endl;
    std::cout << "Pendenza  : " << f0->GetParameter(1) << " +/- " << f0->GetParError(1) << std::endl;
}

// Fit esponenziale: I(V) = I0 * exp(m * V)
void silicio()
{
    const char *filename = "data/dati_silicio.txt";
    TGraphErrors *gr = new TGraphErrors(filename, "%lg %lg %lg %lg");

    if (gr->GetN() == 0)
    {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }

    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);
    gr->SetTitle("Diodo Silicio;Tensione (mV);Corrente (mA)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);

    // Modello di Shockley: I(V) = I0 * (exp(m*V) - 1)
    TF1 *f = new TF1("fshock_si", "[0]*(exp(x/[1])-1)", 0., 1000.);
    f->SetParNames("I0", "etaVt");
    // Valori iniziali ragionevoli per convergenza (eta*Vt ~ 26 mV a T ambiente)
    f->SetParameter(0, 1e-6);
    f->SetParameter(1, 60);
    f->SetLineColor(kRed);

    gr->Fit(f, "QS"); // Q=quiet, S=store result

    double I0 = f->GetParameter(0);
    double etaVt = f->GetParameter(1);
    double sigma_I0 = f->GetParError(0);
    double sigma_etaVt = f->GetParError(1);

    // Disegno (la funzione f disegnata rappresenta il fit eseguito)
    gr->Draw("AP");
    f->Draw("same");
    // Scala logaritmica sull'asse Y (corrente)
    if (gPad)
        gPad->SetLogy(1);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n>>> Silicio (" << filename << ") - fit Shockley (I=I0*(exp(m*V)-1)) :\n";
    std::cout << "I0 (saturazione)      = " << I0 << " +/- " << sigma_I0 << " (mA)" << std::endl;
    std::cout << "eta*Vt (parametro)    = " << etaVt << " +/- " << sigma_etaVt << " (mV)" << std::endl;
}

void germanio()
{
    const char *filename = "data/dati_germanio.txt";
    TGraphErrors *gr = new TGraphErrors(filename, "%lg %lg %lg %lg");

    if (gr->GetN() == 0)
    {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);
    gr->SetTitle("Diodo Germanio;Tensione (mV);Corrente (mA)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kGreen + 2);

    TF1 *f = new TF1("fshock_ge", "[0]*(exp(x/[1])-1)", 0, 1000);
    f->SetParNames("I0", "etaVt");
    f->SetParameter(0, 0.005);
    f->SetParameter(1, 30);
    f->SetLineColor(kRed);

    gr->Fit(f, "QS");

    double I0 = f->GetParameter(0);
    double etaVt = f->GetParameter(1);
    double sigma_I0 = f->GetParError(0);
    double sigma_etaVt = f->GetParError(1);

    gr->Draw("AP");
    
    // Scala logaritmica sull'asse Y (corrente)
    if (gPad)
        gPad->SetLogy(1);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n>>> Germanio (" << filename << ") - fit Shockley (I=I0*(exp(m*V)-1)) :\n";
    std::cout << "I0 (saturazione)      = " << I0 << " +/- " << sigma_I0 << " (mA)" << std::endl;
    std::cout << "eta*Vt (parametro)    = " << etaVt << " +/- " << sigma_etaVt << " (mV)" << std::endl;
}

void analisi_completa()
{
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);

    TCanvas *c1 = new TCanvas("c1", "Analisi Diodi (esponenziale)", 1200, 800);
    c1->Divide(2, 2);

    c1->cd(1);
    fit_calibrazione();

    c1->cd(2);
    silicio();

    c1->cd(3);
    germanio();
}
