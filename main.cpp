#include "SparseRecord.h"
#include "DenseRecord.h"
#include "DenseDataLoader.h"
#include "SparseDataLoader.h"
#include "DataTypeConverter.h"

#include <time.h>
#include <unistd.h>
#include <map>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include <initializer_list>
#include <cstdlib>
#include <string>
#include <cstdio>

using namespace std;

struct k_neighborhood_index {
    vector <DenseRecord> samplePoints;
    vector <list<DenseRecord>> tkn;
    void insert(DenseRecord p, list<DenseRecord> tkN)
    {
        samplePoints.push_back(p);
        tkn.push_back(tkN);
    }
};

struct k_neighborhood_index_sparse {
    vector <SparseRecord> samplePoints;
    vector <list<SparseRecord>> tkn;
    void insert(SparseRecord p, list<SparseRecord> tkN)
    {
        samplePoints.push_back(p);
        tkn.push_back(tkN);
    }
};

struct first_further
{
    float distance_of_e;
    first_further(float distance_of_e) : distance_of_e(distance_of_e)
    {
    }
    bool operator ()(DenseRecord current)
    {
        return distance_of_e < current.distance;
    }
    bool operator ()(SparseRecord current)
    {
        return distance_of_e < current.distance;
    }
};

struct compareRecords
{
    bool operator ()(DenseRecord a, DenseRecord b)
    {
        return a.distance < b.distance;
    }
    bool operator ()(SparseRecord a, SparseRecord b)
    {
        return a.distance < b.distance;
    }
} comparisonFunction;

template <class kni_type, class data_type> void ti_l_neighborhood_index_algorithm(vector<data_type> &D, int k, data_type reference_point, vector<data_type> &s);
template <class data_type> list<data_type> ti_k_neighborhood(vector<data_type> &D, data_type p, int k);
template <class data_type> bool precedingdata_type(vector<data_type> &D, data_type &p);
template <class data_type> bool followingdata_type(vector<data_type> &D, data_type &p);
template <class data_type> void find_first_k_candidate_neighbours_forward(vector<data_type> &D, data_type &p,
        data_type &f, bool &forwardSearch, list<data_type> &k_neighborhood, int k,
        int &i);
template <class data_type> void find_first_k_candidate_neighbours_backward(vector<data_type> &D, data_type &p,
        data_type &b, bool &backwardSearch, list<data_type> &k_neighborhood, int k,
        int &i);
template <class data_type> void find_first_k_candidate_neighbours_forward_and_backward(vector<data_type> &D,
        data_type &p, data_type &b, data_type &f, bool &backwardSearch, bool &forwardSearch,
        list<data_type> &k_neighborhood, int k, int &i);
template <class data_type> void verify_k_candidate_neighbours_forward(vector<data_type> &D, data_type &p, data_type &f,
        bool &forwardSearch, list<data_type> &k_neighborhood, int k);
template <class data_type> void verify_k_candidate_neighbours_backward(vector<data_type> &D, data_type &p,
        data_type &b, bool &backwardSearch, list<data_type> &k_neighborhood, int k);

vector<DenseRecord> createPointVector(DenseDataVector denseDataVector, int decisionColumnNumber);
vector<SparseRecord> createPointVector(SparseDataVector sparseDataVector, int decisionColumnNumber);
vector <string> readDecisionClass(string filename);

vector <string> decisionClass;

int metric;
int decisionClassColumn=0;
int objectNumber;

bool k_plus = false;
bool terminal_output=true;
bool column_decision=false;
bool brute_force=false;
bool talkative=false;
bool forward_heuristic=false;

vector <int> numberOfComparisons;
vector <map <string, int>> choosenClass;
vector <map <string, int>> choosenClassTerminal;


int main(int argc, char* argv[])
{
    metric=2;

    time_t t;
    srand((unsigned) time(&t));
    float startLoadTime = (float)clock()/CLOCKS_PER_SEC;

    enum {undefined=-1};
    int k = undefined;

    DenseDataLoader sample_loader;
    DenseDataLoader dataset_loader;

    SparseDataLoader sample_loader_sparse;
    SparseDataLoader dataset_loader_sparse;

    bool swap_data = false;
    bool sample_defined=false;
    bool dataset_defined=false;
    bool file_decision=false;
    bool sparse_data=false;

    string fileData;
    string fileSample;

    int c;
    while((c = getopt(argc, argv,"D:S:cd:n:ptom:k:bfs")) != -1)
    {
        switch(c)
        {
            case 'D':
                if(optarg)
                {
                    cout << "Setup: Reading dataset D from file: " << optarg << endl;
                    fileData = optarg;
                    dataset_defined=true;
                }
                break;
            case 'S':
                if(optarg)
                {
                    cout << "Setup: Reading sample from file: " << optarg << endl;
                    fileSample = optarg;
                    sample_defined=true;
                }
                break;
            case 'c':
                cout << "Setup: Data converted" << endl;
                swap_data=true;
                break;
            case 'd':
                if(optarg)
                {
                    cout << "Setup: Reading decision class from file: " << optarg << endl;
                    decisionClass=readDecisionClass(optarg);
                    file_decision=true;
                }
                break;
            case 'n':
                if(optarg)
                {
                    cout << "Setup: Reading decision class from column: " << optarg << endl;
                    column_decision=true;
                    decisionClassColumn=atoi(optarg);
                }
                break;
            case 'p':
                cout << "Setup: Using k+ algorithm" << endl;
                k_plus = true;
                break;
            case 't':
                cout << "Setup: Talkative mode" << endl;
                talkative = true;
                break;
            case 'o':
                cout << "Setup: Terminal output suspended" << endl;
                terminal_output=false;
                break;
            case 'm':
                if(optarg)
                {
                    metric=atoi(optarg);
                    if(metric<=0)
                        metric=2;
                    cout << "Setup: Metric = " << metric << endl;
                }
                break;
            case 'k':
                if(optarg)
                {
                    k=atoi(optarg);
                    if(k<0)
                        k=3;
                    cout << "Setup: k=" << k << endl;
                }
                break;
            case 'b':
                cout << "Setup: Brute force mode" << endl;
                brute_force=true;
                break;
            case 'f':
                cout << "Setup: Using forward heuristic"  << endl;
                forward_heuristic=true;
                break;
            case 's':
                cout << "Setup: Sparse data" << endl;
                sparse_data=true;
                break;
        }
    }

    if(1==argc || undefined == k)
    {
        cout << "Setup: Using default settings (k algorithm, k=3)" << endl;
        k=3;
    }
    if(!forward_heuristic)
    {
        cout << "Setup: Using backward heuristic"  << endl;
    }
    if(!dataset_defined)
    {
        if(!sparse_data)
            dataset_loader.readDenseData("dense.csv");
        else
            dataset_loader_sparse.readSparseData("cranmed.mat");
    }
    else
    {
        if(!sparse_data)
            dataset_loader.readDenseData(fileData);
        else
            dataset_loader_sparse.readSparseData(fileData);
    }
    if(!sample_defined)
    {
        if(!sparse_data)
            sample_loader.readDenseData("sample.csv");
        else
            sample_loader_sparse.readSparseData("cranmedSamples.csv");
    }
    else
    {
        if(!sparse_data)
            sample_loader.readDenseData(fileSample);
        else
        {
            sample_loader_sparse.readSparseData(fileSample);
        }
    }
    if(!file_decision&&decisionClassColumn<1)
    {
        if(!sparse_data)
            decisionClass=readDecisionClass("classes.csv");
        else
            decisionClass=readDecisionClass("cranmed.mat.rclass");
    }
    if(column_decision)
    {
        decisionClass=readDecisionClass(fileData);
    }
    if(swap_data)
    {
        DataTypeConverter converter;
        int max_index;
        if(sparse_data)
        {
            if(dataset_loader_sparse.maxIndex>dataset_loader.maxIndex)
                max_index=dataset_loader_sparse.maxIndex;
            else
                max_index=dataset_loader.maxIndex;
        }
        if(sparse_data)
        {
            dataset_loader.denseDataVector = converter.sparseToDense(dataset_loader_sparse.rowVector,max_index);
            sample_loader.denseDataVector  = converter.sparseToDense(sample_loader_sparse.rowVector,max_index);
        }
        else
        {
            dataset_loader_sparse.rowVector = converter.denseToSparse(dataset_loader.denseDataVector);
            sample_loader_sparse.rowVector  = converter.denseToSparse(sample_loader.denseDataVector);
        }
    }

    vector<DenseRecord> D = createPointVector(dataset_loader.denseDataVector,decisionClassColumn);
    vector<DenseRecord> s = createPointVector(sample_loader.denseDataVector, decisionClassColumn);
    vector<SparseRecord> D_sparse = createPointVector(dataset_loader_sparse.rowVector,decisionClassColumn);
    vector<SparseRecord> s_sparse = createPointVector(sample_loader_sparse.rowVector, decisionClassColumn);

    if(D.size()*s.size()==0&&D_sparse.size()*s_sparse.size()==0)
    {
        cout << "Error while reading data" << endl;
        return -1;
    }

    float endLoadTime = (float)clock()/CLOCKS_PER_SEC;
    float startAlgorithmTime = (float)clock()/CLOCKS_PER_SEC;
    if(sparse_data)
    {
        if(swap_data!= true)
        {
            pair<int, float> init;
            init.first=0;
            init.second=0;
            vector <pair<int, float>> origo;
            origo.push_back(init);
            ti_l_neighborhood_index_algorithm<k_neighborhood_index_sparse, SparseRecord>(D_sparse, k, SparseRecord(origo, 0, 0), s_sparse);
        }
        else
        {
            vector <float> origo;
            origo.resize(D[0].parameters.size());
            ti_l_neighborhood_index_algorithm<k_neighborhood_index, DenseRecord>(D, k, DenseRecord(origo, 0, 0), s);
        }
    }
    else
    {
        if(swap_data != true)
        {
            vector <float> origo;
            origo.resize(D[0].parameters.size());
            ti_l_neighborhood_index_algorithm<k_neighborhood_index, DenseRecord>(D, k, DenseRecord(origo, 0, 0), s);
        }
        else
        {
            pair<int, float> init;
            init.first=0;init.second=0;
            vector <pair<int, float>> origo;
            origo.push_back(init);
            ti_l_neighborhood_index_algorithm<k_neighborhood_index_sparse, SparseRecord>(D_sparse, k, SparseRecord(origo, 0, 0), s_sparse);
        }
    }

    float endAlgorithmTime = (float)clock()/CLOCKS_PER_SEC;
    cout << endl;
    cout << "Data loaded in: " << endLoadTime-startLoadTime << endl;
    cout << "Algoritm processed in: " << endAlgorithmTime-startAlgorithmTime << endl;

    std::fstream plik;
    plik.open( "output.txt", ios::out | ios::app );
    if( plik.good() == true )
    {
        plik << "Data loaded in: " << endLoadTime-startLoadTime << endl;
        plik << "Algoritm processed in: " << endAlgorithmTime-startAlgorithmTime << endl;
    }
    else
        cout << "Dostep do pliku zostal zabroniony!" << endl;
    return 0;
}


template <class kni_type, class data_type>
void ti_l_neighborhood_index_algorithm(vector<data_type> &D, int k, data_type reference_point, vector<data_type> &s)
{
    kni_type kni;
    int number_of_points_in_D = D.size();
    if(terminal_output)
        cout << "Dataset: " << endl;
    for(int i = 0; i<number_of_points_in_D; ++i)
    {
        if(!brute_force)
            D[i].distance = D[i].calculateDistance(reference_point,metric);
        D[i].id=i+1;
        if(terminal_output)
            cout << i << "\t" << D[i] << endl;
    }
    if(terminal_output)
        cout << "Samples: " << endl;
    int number_of_points_in_s = s.size();
    data_type **samples = new data_type*[number_of_points_in_s];
    for(int i = 0; i<number_of_points_in_s; ++i)
    {
        samples[i] = &s[i];
        s[i].distance = s[i].calculateDistance(reference_point,metric);
        s[i].id=-1-i;
        if(terminal_output)
            cout << i << "\t" << s[i] << endl;
    }
    choosenClass.resize(number_of_points_in_s);
    choosenClassTerminal.resize(number_of_points_in_s);
    typename vector<data_type>::iterator sample;
    for(int i=0; i<number_of_points_in_s; ++i)
    {
        objectNumber=i;
        D.insert(D.end(),(s.begin()+i),(s.begin()+i+1));
        std::sort(D.begin(),D.end(),comparisonFunction);
        int curr_number_of_points_in_D=D.size();
        for(int i = 0; i<curr_number_of_points_in_D; ++i)
        {
            D[i].position=i;
        }
        sample=find(D.begin(),D.end(),*samples[i]);
        kni.insert((*sample), ti_k_neighborhood<data_type>(D,(*sample),k));
        D.erase(sample);
        if(100*i/number_of_points_in_s%2==0)
        {
            cout << "Processing: " << 100*i/number_of_points_in_s << "%";
            if(100*i/number_of_points_in_s<10)
                cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
            else if (100*i/number_of_points_in_s<100)
                cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
            else
                cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
        }
    }
    cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
    cout << "Processing finished!" << endl;
    if(terminal_output)
    {
        cout << endl;

        cout << "Results" << endl;
        if(!talkative)
            cout << "Object:\n\tDist:\t<NN id> & NN classes:" << endl;
        else
            cout << "Object:\n\tDist:\t<NN id> & NN parameters & NN classes:" << endl;
        cout << endl;
        int ii=0;
        for(typename vector < list<data_type> >::iterator it = kni.tkn.begin(); it != kni.tkn.end(); ++it)
        {
           cout << ii << "# " << s[ii] << endl;
           (*it).sort(comparisonFunction);
           for(typename list<data_type>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
           {
                if(talkative)
                {
                    cout << "\t" << it2->distance << "\t" << *it2 << "\t";
                }
                else
                {
                    cout << "\t" << it2->distance << "\t" <<= *it2; cout << "\t";
                }
                cout<< decisionClass[(*it2).id] <<endl;
                choosenClassTerminal[ii][decisionClass[(*it2).id]]+=1;
           }
           map<string,int>::iterator it3;
           cout << "Occurances of neighbours' classes: " << endl;
           int best_score =0;
           string best_class;
           for(it3=choosenClassTerminal[ii].begin();it3!=choosenClassTerminal[ii].end();++it3)
           {
               cout << (*it3).first << " " << (*it3).second << endl;
               if((*it3).second>best_score)
               {
                   best_score=(*it3).second;
                   best_class=(*it3).first;
               }
           }
           cout << "Choosen class: " << best_class << endl;
           cout << "Number of comparisons: " << numberOfComparisons[ii] << endl;
           cout << endl;
           ii++;
        }
        cout << endl;
    }
    std::fstream plik;
    plik.open( "output.txt", std::ios::out );
    if( plik.good() == true )
    {
        plik << "Results" << endl;
        if(!talkative)
            plik << "Object:\n\tDist:\t<NN id> & NN classes:" << endl;
        else
            plik << "Object:\n\tDist:\t<NN id> & NN parameters & NN classes:" << endl;
        plik << endl;
        int ii=0;
        for(typename vector < list<data_type> >::iterator it = kni.tkn.begin(); it != kni.tkn.end(); ++it)
        {
            plik << ii << "#" << s[ii] << endl;
            (*it).sort(comparisonFunction);
            for(typename list<data_type>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2){
                if(talkative)
                {
                    plik << "\t" << it2->distance << "\t" << *it2 << "\t";
                }
                else
                {
                    plik << "\t" << it2->distance << "\t" <<= *it2; plik << "\t";
                }
                plik << decisionClass[(*it2).id] <<endl;
                choosenClass[ii][decisionClass[(*it2).id]]+=1;
            }
            map<string,int>::iterator it3;
            plik << "Occurances of neighbours' classes: " << endl;
            int best_score =0;
            string best_class;
            for(it3=choosenClass[ii].begin();it3!=choosenClass[ii].end();++it3)
            {
                plik << (*it3).first << " " << (*it3).second << endl;
                if((*it3).second>best_score)
                {
                    best_score=(*it3).second;
                    best_class=(*it3).first;
                }
            }
            plik << "Choosen class: " << best_class << endl;
            plik << "Number of comparisons: " << numberOfComparisons[ii] << endl;
            plik << endl;
            ii++;
        }
        plik << endl;
        plik.close();
    }
    else
        cout << "Dostep do pliku zostal zabroniony!" << endl;
}

template <class data_type>
list<data_type> ti_k_neighborhood(vector<data_type> &D, data_type p, int k)
{
    numberOfComparisons.push_back(0);
    list<data_type> k_neighborhood = {};
    if(brute_force)
    {
        int size_of_D = D.size();
        for(int i = 0; i<size_of_D; ++i)
        {
            D[i].distance = D[i].calculateDistance(p,metric);
        }
        numberOfComparisons[objectNumber]=size_of_D-1;
        std::sort(D.begin(),D.end(),comparisonFunction);
        for(int i =0; i<k+1; i++)
        {
            if(D[i].id!=p.id && k<size_of_D-1)
                k_neighborhood.push_back(D[i]);
        }
        ++k;
        while(k<size_of_D-1 && k_plus)
        {
            if(k_neighborhood.back().distance==D[k].distance)
            {
                k_neighborhood.push_back(D[k]);
                ++k;
            }
            else
               break;
        }
        return k_neighborhood;
    }
        //cout << "list<data_type> ti_k_neighborhood(vector<data_type> &D, data_type p, int k)" << endl;
    //cout << "data_type p: " << p;
    data_type b = p;
    data_type f = p;

    bool backwardSearch = precedingPoint(D, b);
    bool forwardSearch = followingPoint(D, f);


    int i = 0;
    typename list<data_type>::iterator it;

    find_first_k_candidate_neighbours_forward_and_backward(D, p, b, f, backwardSearch, forwardSearch, k_neighborhood, k, i);
    if(!forward_heuristic)
    {
        find_first_k_candidate_neighbours_backward(D, p, b, backwardSearch, k_neighborhood, k, i);
        find_first_k_candidate_neighbours_forward(D, p, f, forwardSearch, k_neighborhood, k, i);
    }
    else
    {
        find_first_k_candidate_neighbours_forward(D, p, f, forwardSearch, k_neighborhood, k, i);
        find_first_k_candidate_neighbours_backward(D, p, b, backwardSearch, k_neighborhood, k, i);
    }
    p.eps = (*k_neighborhood.begin()).distance;
    for(it=k_neighborhood.begin();it!=k_neighborhood.end();++it)
        if(p.eps<(*it).distance)
            p.eps=(*it).distance;

    if(!forward_heuristic)
    {
        verify_k_candidate_neighbours_backward(D, p, b, backwardSearch, k_neighborhood, k);
        verify_k_candidate_neighbours_forward(D, p, f, forwardSearch, k_neighborhood, k);
    }
    else
    {
        verify_k_candidate_neighbours_forward(D, p, f, forwardSearch, k_neighborhood, k);
        verify_k_candidate_neighbours_backward(D, p, b, backwardSearch, k_neighborhood, k);
    }
    return k_neighborhood;
}
template <class data_type>
bool precedingPoint(vector<data_type> &D, data_type &p)
{
    if(p.position>0)
    {
        p=D[p.position-1];
        return true;
    }
    else
         return false;
}
template <class data_type>
bool followingPoint(vector<data_type> &D, data_type &p)
{
    if(p.position!=D.size()-1)
    {
        p=D[p.position+1];
        return true;
    }
    else
         return false;
}
template <class data_type>
void find_first_k_candidate_neighbours_forward_and_backward(vector<data_type> &D, data_type &p, data_type &b, data_type &f,
                                                            bool &backwardSearch, bool &forwardSearch,
                                                            list<data_type> &k_neighborhood, int k, int &i)
{
    //cout << "void find_first_k_candidate_neighbours_forward_and_backward" << endl;
    float distance;
    while(backwardSearch && forwardSearch && i<k)
    {
        if(!forward_heuristic)
        {
            if(p.distance - b.distance < f.distance - p.distance)
            {
                distance = b.calculateDistance(p,metric);
                i = i+1;
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                backwardSearch=precedingPoint(D,b);
            }
            else
            {
                distance = f.calculateDistance(p,metric);
                i = i+1;
                data_type e = data_type(f.parameters, distance, f.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                forwardSearch=followingPoint(D,f);
            }
        }
        else
        {
            if(p.distance - b.distance > f.distance - p.distance)
            {
                distance = f.calculateDistance(p,metric);
                i = i+1;
                data_type e = data_type(f.parameters, distance, f.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                forwardSearch=followingPoint(D,f);
            }
            else
            {
                distance = b.calculateDistance(p,metric);
                i = i+1;
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                backwardSearch=precedingPoint(D,b);
            }
        }
    }
}
template <class data_type>
void find_first_k_candidate_neighbours_backward(vector<data_type> &D, data_type &p, data_type &b,
                                                bool &backwardSearch, list<data_type> &k_neighborhood, int k, int &i)
{
    //cout << "void find_first_k_candidate_neighbours_backward" << endl;
    float distance;
    while(backwardSearch && i<k)
    {
        distance = b.calculateDistance(p,metric);
        i = i+1;
        data_type e = data_type(b.parameters, distance, b.id);
        typename list<data_type>::iterator it;
        it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
        k_neighborhood.insert(it, e);
        backwardSearch=precedingPoint(D,b);
    }
}
template <class data_type>
void find_first_k_candidate_neighbours_forward(vector<data_type> &D, data_type &p, data_type &f,
                                               bool &forwardSearch, list<data_type> &k_neighborhood, int k, int &i)
{
    //cout << "void find_first_k_candidate_neighbours_forward" << endl;
    float distance;
    while(forwardSearch && i<k)
    {
        distance = f.calculateDistance(p,metric);
        i = i+1;
        data_type e = data_type(f.parameters, distance, f.id);
        typename list<data_type>::iterator it;
        it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
        k_neighborhood.insert(it, e);
        forwardSearch=followingPoint(D,f);
    }
}
template <class data_type>
void verify_k_candidate_neighbours_backward(vector<data_type> &D, data_type &p, data_type &b, bool &backwardSearch,
                                            list<data_type> &k_neighborhood, int k)
{
    while (backwardSearch && ((p.distance - b.distance) <= p.eps))
    {
        numberOfComparisons[objectNumber]++;
        float distance = b.calculateDistance(p,metric);
        if (distance < p.eps)
        {
            int edge_data_types = 0;
            typename list<data_type>::iterator low = k_neighborhood.begin();
            typename list<data_type>::iterator up = k_neighborhood.begin();
            typename list<data_type>::iterator it;
            for(it = k_neighborhood.begin(); it!=k_neighborhood.end(); ++it)
            {
                if((*it).distance == p.eps)
                {
                    if(edge_data_types == 0)
                        low=it;
                    edge_data_types++;
                }
                else if(edge_data_types > 0)
                {
                    up = it;
                    break;
                }
                if(it==--k_neighborhood.end())
                {
                    up = k_neighborhood.end();
                    break;
                }
            }
            //cout << k_neighborhood.size() << " - " << edge_data_types << " >= " << k-1 << endl;
            if(k_neighborhood.size() - edge_data_types >= k - 1)
            {
                if(1 < edge_data_types)
                    k_neighborhood.erase(low,up);
                if(1 == edge_data_types)
                    k_neighborhood.erase(low);
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                p.eps = (*(--k_neighborhood.end())).distance;
            }
            else
            {
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                if(k_plus != true && k_neighborhood.size() > k)
                {
                    k_neighborhood.erase(--k_neighborhood.end());
                }
            }
        }
        else if(distance == p.eps)
        {
            if(true == k_plus || k_neighborhood.size() < k)
            {
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
            }
        }
        backwardSearch = precedingPoint(D, b);
    }
}
template <class data_type>
void verify_k_candidate_neighbours_forward(vector<data_type> &D, data_type &p, data_type &f, bool &forwardSearch,
                                           list<data_type> &k_neighborhood, int k)
{
    while (forwardSearch && ((f.distance - p.distance) <= p.eps))
    {
        numberOfComparisons[objectNumber]++;
        float distance = f.calculateDistance(p,metric);
        if (distance < p.eps)
        {
            int edge_data_types = 0;
            typename list<data_type>::iterator low = k_neighborhood.begin();
            typename list<data_type>::iterator up = k_neighborhood.begin();
            typename list<data_type>::iterator it;
            for(it = k_neighborhood.begin(); it!=k_neighborhood.end(); ++it)
            {
                if((*it).distance == p.eps)
                {
                    if(edge_data_types == 0)
                    {
                        low=it;
                    }
                    edge_data_types++;
                }
                else if(edge_data_types > 0)
                {
                    up = it;
                    break;
                }
                if(it==--k_neighborhood.end())
                {
                    up = k_neighborhood.end();
                    break;
                }
            }
            //cout << k_neighborhood.size() << " - " << edge_data_types << " >= " << k-1 << endl;
            if(k_neighborhood.size() - edge_data_types >= k - 1)
            {
                if(1 < edge_data_types)
                {
                    k_neighborhood.erase(low,up);
                }
                if(1 == edge_data_types)
                    k_neighborhood.erase(low);

                data_type e = data_type(f.parameters, distance, f.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                p.eps = (*(--k_neighborhood.end())).distance;
            }
            else
            {
                data_type e = data_type(f.parameters, distance, f.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
                if(k_plus != true && k_neighborhood.size() > k)
                {
                    k_neighborhood.erase(--k_neighborhood.end());
                }
            }
        }
        else if(distance == p.eps)
        {
            if(true == k_plus || k_neighborhood.size() < k)
            {
                data_type e = data_type(f.parameters, distance, f.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), first_further(e.distance));
                k_neighborhood.insert(it, e);
            }
        }
        forwardSearch = followingPoint(D, f);
    }
}

vector<DenseRecord> createPointVector(DenseDataVector denseDataVector, int decisionColumnNumber)
{
    vector <DenseRecord> D;
    DenseRecord tmp;
    int i=0;
    for(DenseDataVector::iterator it = denseDataVector.begin(); it != denseDataVector.end(); ++it)
    {
        tmp.parameters.clear();
        for(vector<float>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2){
            i++;
            if(i != decisionColumnNumber)
            {
                tmp.parameters.push_back(*it2);
            }
        }
        D.push_back(tmp);
        i=0;
    }
    return D;
}

vector<SparseRecord> createPointVector(SparseDataVector sparseDataVector, int decisionColumnNumber)
{
    auto sortSparse = [](const std::pair<int,float> &left, const std::pair<int,float> &right) { return left.first < right.first;};
    for(SparseDataVector::iterator it = sparseDataVector.begin(); it!=sparseDataVector.end(); ++it)
        std::sort((*it).begin(),(*it).end(),sortSparse);

    vector <SparseRecord> D;
    SparseRecord tmp;
    for(SparseDataVector::iterator it = sparseDataVector.begin(); it != sparseDataVector.end(); ++it)
    {
        tmp.parameters.clear();
        for(vector<pair<int,float>>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2){
            if((*it2).first != decisionColumnNumber)
            {
                tmp.parameters.push_back(*it2);
            }
        }
        D.push_back(tmp);
    }
    return D;
}

vector <string> readDecisionClass(string filename)
{
    vector <string> decisionClass;
    ifstream file (filename);
    string line;
    getline(file,line);
    int row = 0;
    int col = 0;
    while(getline(file,line))
    {
        std::stringstream   linestream(line);
        std::string value;
        decisionClass.resize(row + 1);
        while(getline(linestream,value,','))
        {
            if(column_decision)
            {
                ++col;
                if(decisionClassColumn!=col)
                    continue;
            }
            try{
                decisionClass.push_back(value);
            } catch (const std::invalid_argument&) {
                decisionClass.push_back(0);
            }
        }
        col=0;
        ++row;
    }
    file.close();
    return decisionClass;
}
