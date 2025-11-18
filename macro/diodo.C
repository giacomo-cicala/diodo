
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TAxis.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TMath.h"
#include <iostream>

// --- 1. Funzione Calibrazione ---
void fit_calibrazione() {
    const char* filename = "dati_calibrazione.txt";
    // Il costruttore legge direttamente il file.
    // Formato: x y ex sigma_y
    TGraphErrors* gr = new TGraphErrors(filename, "%lg %lg %lg %lg");

    if (gr->GetN() == 0) {
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

    std::cout << "\n>>> Calibrazione (" << filename << " ):\n";
    std::cout << "Intercetta: " << f0->GetParameter(0) << " +/- " << f0->GetParError(0) << std::endl;
    std::cout << "Pendenza  : " << f0->GetParameter(1) << " +/- " << f0->GetParError(1) << std::endl;
}

// --- 2. Funzione Silicio ---
void silicio() {
    const char* filename = "dati_silicio.txt";
    // Per file con solo X e Y: leggiamo e trasformiamo y->ln(y)
    TGraphErrors* gr = new TGraphErrors(filename, "%lg %lg");

    if (gr->GetN() == 0) {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }

    gr->SetTitle("Diodo Silicio;Tensione (mV);Corrente (mA)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlue);

    // Costruiamo un grafico dei log: Y' = ln(Y), sigma_Y' = sigma_Y / Y
    int n = gr->GetN();
    TGraphErrors* gl = new TGraphErrors();
    int k = 0;
    for (int i = 0; i < n; ++i) {
        double x, y;
        gr->GetPoint(i, x, y);
        double sigma_y = gr->GetErrorY(i);
        if (y > 0 && std::isfinite(y)) {
            double logy = log(y);
            double sigma_logy = sigma_y / y;
            gl->SetPoint(k, x, logy);
            gl->SetPointError(k, gr->GetErrorX(i), sigma_logy);
            ++k;
        }
    }

    if (k == 0) {
        std::cout << "Errore: nessun punto valido per il log in " << filename << std::endl;
        return;
    }

    gl->SetMarkerStyle(20);
    gl->SetMarkerColor(kBlue);

    // Fit lineare: ln(I) = a + b*x
    TF1 *fl = new TF1("flin", "[0]+[1]*x", 0., 1000.);
    fl->SetLineColor(kRed);
    TFitResultPtr r = gl->Fit(fl, "QS"); // Q=quiet S=store result

    // Coefficienti della retta
    double a = fl->GetParameter(0);
    double b = fl->GetParameter(1);
    double sigma_a = fl->GetParError(0);
    double sigma_b = fl->GetParError(1);

    // Recupera matrice di covarianza se disponibile
    TMatrixD cov(2,2);
    if (r.Get() && r->GetCovarianceMatrix()) {
        TMatrixD *pcov = r->GetCovarianceMatrix();
        cov = *pcov;
    }
    // Calcoli dei parametri fisici:
    // relazione: a = -etaVt * ln(I0)  =>  I0 = exp(-a/etaVt)
    // trattiamo etaVt come il parametro di slope b (etaVt = b)
    double etaVt = b;
    double sigma_etaVt = sigma_b;

    double I0 = 0.0;
    double sigma_I0 = 0.0;
    if (etaVt != 0.0) {
        I0 = exp(-a / etaVt);
        double dI0_da = (1.0 / etaVt) * I0;
        double dI0_db = (a / (etaVt * etaVt)) * I0;
        double var_I0 = dI0_da * dI0_da * cov(0,0)
                        + dI0_db * dI0_db * cov(1,1)
                        + 2.0 * dI0_da * dI0_db * cov(0,1);
        sigma_I0 = (var_I0 > 0.0) ? sqrt(var_I0) : 0.0;
    }

    // Draw original and log-fit
    gl->Draw("AP");

    std::cout << "\n>>> Silicio (" << filename << ") - fit su ln(I):\n";
    std::cout << "a (intercetta lnI) = " << a << " +/- " << sigma_a << std::endl;
    std::cout << "b (pendenza)        = " << b << " +/- " << sigma_b << std::endl;
    std::cout << "I0 = exp(-a/etaVt) = " << I0 << " +/- " << sigma_I0 << std::endl;
    std::cout << "eta*Vt = b = " << etaVt << " +/- " << sigma_etaVt << std::endl;
}

// --- 3. Funzione Germanio ---
void germanio() {
    const char* filename = "dati_germanio.txt";
    // Per file con solo X e Y: trasformiamo y->ln(y) e fit lineare
    TGraphErrors* gr = new TGraphErrors(filename, "%lg %lg");

    if (gr->GetN() == 0) {
        std::cout << "Errore: " << filename << " vuoto o non trovato!" << std::endl;
        return;
    }

    gr->SetTitle("Diodo Germanio;Tensione (mV);Corrente (mA)");
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kGreen+2);

    int n = gr->GetN();
    TGraphErrors* gl = new TGraphErrors();
    int k = 0;
    for (int i = 0; i < n; ++i) {
        double x, y;
        gr->GetPoint(i, x, y);
        double sigma_y = gr->GetErrorY(i);
        if (y > 0 && std::isfinite(y)) {
            double logy = log(y);
            double sigma_logy = sigma_y / y;
            gl->SetPoint(k, x, logy);
            gl->SetPointError(k, gr->GetErrorX(i), sigma_logy);
            ++k;
        }
    }

    if (k == 0) {
        std::cout << "Errore: nessun punto valido per il log in " << filename << std::endl;
        return;
    }

    gl->SetMarkerStyle(20);
    gl->SetMarkerColor(kGreen+2);

    TF1 *fl = new TF1("flin_ge", "[0]+[1]*x", 0., 1000.);
    fl->SetLineColor(kRed);
    TFitResultPtr r = gl->Fit(fl, "QS");

    double a = fl->GetParameter(0);
    double b = fl->GetParameter(1);
    double sigma_a = fl->GetParError(0);
    double sigma_b = fl->GetParError(1);

    TMatrixD cov(2,2);
    if (r.Get() && r->GetCovarianceMatrix()) {
        TMatrixD *pcov = r->GetCovarianceMatrix();
        cov = *pcov;
    } else {
        cov(0,0) = sigma_a*sigma_a; cov(1,1) = sigma_b*sigma_b; cov(0,1) = cov(1,0) = 0.0;
    }

    double etaVt = b;
    double sigma_etaVt = sigma_b;

    double I0 = 0.0;
    double sigma_I0 = 0.0;
    if (etaVt != 0.0) {
        I0 = exp(-a / etaVt);
        double dI0_da = (1.0 / etaVt) * I0;
        double dI0_db = (a / (etaVt * etaVt)) * I0;
        double var_I0 = dI0_da * dI0_da * cov(0,0)
                        + dI0_db * dI0_db * cov(1,1)
                        + 2.0 * dI0_da * dI0_db * cov(0,1);
        sigma_I0 = (var_I0 > 0.0) ? sqrt(var_I0) : 0.0;
    }

    gl->Draw("AP");

    std::cout << "\n>>> Germanio (" << filename << ") - fit su ln(I):\n";
    std::cout << "a (intercetta lnI) = " << a << " +/- " << sigma_a << std::endl;
    std::cout << "b (pendenza)        = " << b << " +/- " << sigma_b << std::endl;
    std::cout << "I0 = exp(-a/etaVt) = " << I0 << " +/- " << sigma_I0 << std::endl;
    std::cout << "eta*Vt = b = " << etaVt << " +/- " << sigma_etaVt << std::endl;
}


// --- MAIN ---
void analisi_completa() {
    // Setup stile globale
    gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0);

    // Canvas divisa
    TCanvas* c1 = new TCanvas("c1", "Analisi Diodi", 1200, 800);
    c1->Divide(2, 2);

    // Chiama le funzioni passando i nomi dei file
    c1->cd(1);
    fit_calibrazione();

    c1->cd(2);
    silicio();

    c1->cd(3);
    germanio();

    // Opzionale: Salvataggio immagine
    // c1->SaveAs("risultati_analisi.png");
}