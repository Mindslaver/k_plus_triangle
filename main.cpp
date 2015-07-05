/**
*
* Author: Adam Anrzejczak
* Date: 2015-01-15
*
* Description:  Implementation of a k+ neighbours algorithm with a triangle inequality optimization
*               for both sparse and dense data.
*
* References:
*
*   "A Neighborhood-Based Clustering by Means of the Triangle Inequality."
*   Marzena Kryszkiewicz and Piotr Lasek.
*   Institute of Computer Science, Warsaw University of Technology.
*
*   Abstract:
*   Grouping data into meaningful clusters is an important task of both artificial
*   intelligence and data mining. An important group of clustering algorithms are
*   density based ones that require calculation of a neighborhood of a given data
*   record. The bottleneck for such algorithms are high dimensional data. In this
*   paper, we propose a new TI-k-Neighborhood-Index algorithm that calculates
*   k-neighborhoods for all records in a given data set by means the triangle
*   inequality. We prove experimentally that the NBC (Neighborhood Based
*   Clustering) clustering algorithm supported by our index outperforms NBC
*   supported by such known spatial indices as VA-file and R-tree both in the case
*   of low and high dimensional data.
*
*
*
*   Usage example:
*    -k 6 -p -D dense.csv -O object.csv -n 2 -o -m 2 -f -c
*
*   Parameters:
*    -k <number of neighbours>
*    -D <path to the database file>
*    -O <path to the file with objects to classify>
*    -d <path to the file with classes names>
*    -n <column that stores class name; counted from 1 upwards>
*    -m <metric; 1=manhattan, 2=euclidean>
*
*   Switches:
*    -p using k+ modification; off by default
*    -s indication, that data records are of the sparse format
*    -o suppresses terminal output (except for setup and log messages)
*    -c treat dense data as sparse or sparse data as dense - not recommended
*    -b brute force, namely no triangle optimization (~1000x slower)
*    -t more detailed output
*    -f forward heuristic, backward by default
*
*   Output file is called output.txt and is overwritten with every run.
*
*/

#include "SparseRecord.h"
#include "DenseRecord.h"
#include "DenseDataLoader.h"
#include "SparseDataLoader.h"
#include "DataTypeConverter.h"

#include <time.h>
#include <unistd.h>
#include <map>
#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include <initializer_list>
#include <string>

using namespace std;

/**
 *  Structures that store the lists of the k nearest neighbours for the objects that will be classified.
 *  They will be assigned to the class that is the most frequent for the respective k neighbours set.
 */
struct k_neighborhood_index
{
    vector <DenseRecord> objectRecords;
    vector <list<DenseRecord>> tkn;
    void insert(DenseRecord p, list<DenseRecord> tkN)
    {
        objectRecords.push_back(p);
        tkn.push_back(tkN);
    }
};
struct k_neighborhood_index_sparse
{
    vector <SparseRecord> objectRecords;
    vector <list<SparseRecord>> tkn;
    void insert(SparseRecord p, list<SparseRecord> tkN)
    {
        objectRecords.push_back(p);
        tkn.push_back(tkN);
    }
};

/**
 *  Structure used for defining comparison between epsilon distance (worst-case heuristic) and real distance from the object of classification.
 */
struct firstFurther
{
    float distance_of_e;
    firstFurther(float distance_of_e) : distance_of_e(distance_of_e)
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

/**
 *  Structure used for defining comparison between two records.
 */
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

//The algorithm for finding k nearest neighbours
template <class data_type> list<data_type> ti_k_neighborhood(vector<data_type> &D, data_type p, int k);
//Function that runs the algorithm for every object and classifies them according to their k neighbours sets
template <class kni_type, class data_type> void ti_l_neighborhood_index_algorithm(vector<data_type> &D, int k, data_type reference_record, vector<data_type> &O);

//Functions used by ti_k_neighborhood algorithm. Their names are self explanatory IFF you read the article given in references.
//Unfortunately, I am not autorised to post anything but its abstract.
template <class data_type> bool precedingRecord(vector<data_type> &D, data_type &p);
template <class data_type> bool followingRecord(vector<data_type> &D, data_type &p);
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

vector<DenseRecord> createRecordVector(DenseDataVector denseDataVector, int decisionColumnNumber);
vector<SparseRecord> createRecordVector(SparseDataVector sparseDataVector, int decisionColumnNumber);
vector <string> readDecisionClass(string filename);
vector <string> decisionClass;

//The exponent value used for calculating the distance between two records i.e. dist = pow[((x1-y1)^metric),metric^-1]
int metric;

//If the decision class is specified as one of the records' parameter its column number is stored here
int decisionClassColumn=0;

//The id of the object for which we are looking for k/k+ nearest neighbours (if no object file is specified)
int objectNumber;

//K+ switch, K by default (so even if there is K+n records that are equally distant from the object, only K will be considered)
bool k_plus = false;

//Output is presented in the terminal by default
bool terminal_output=true;

bool column_decision=false;

//No triangle inequality property taken into accout
bool brute_force=false;

//If the output should contain records parameters
bool talkative=false;

//Heuristis mode, backward by default
bool forward_heuristic=false;

//Number of comparisions between a given object and the dataset
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

    DenseDataLoader object;
    DenseDataLoader dataset;
    SparseDataLoader objectSparse;
    SparseDataLoader datasetSparse;

    bool swap_data = false;
    bool object_defined=false;
    bool dataset_defined=false;
    bool file_decision=false;
    bool sparse_data=false;

    string fileData;
    string fileObject;

    int c;
    while((c = getopt(argc, argv,"D:O:cd:n:ptom:k:bfs")) != -1)
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
            case 'O':
                if(optarg)
                {
                    cout << "Setup: Reading object from file: " << optarg << endl;
                    fileObject = optarg;
                    object_defined=true;
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
            dataset.readDenseData("dense.csv");
        else
            datasetSparse.readSparseData("cranmed.mat");
    }
    else
    {
        if(!sparse_data)
            dataset.readDenseData(fileData);
        else
            datasetSparse.readSparseData(fileData);
    }
    if(!object_defined)
    {
        if(!sparse_data)
            object.readDenseData("object.csv");
        else
            objectSparse.readSparseData("cranmedobjects.csv");
    }
    else
    {
        if(!sparse_data)
            object.readDenseData(fileObject);
        else
        {
            objectSparse.readSparseData(fileObject);
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
    if(!file_decision && !column_decision)
    {
        cout << "No decision factor specified (-d <file> or -n <column>)" << endl;
        return -1;
    }
    if(swap_data)
    {
        //DataTypeConverter converter;
        int max_index;
        if(sparse_data)
        {
            if(datasetSparse.maxIndex>dataset.maxIndex)
                max_index=datasetSparse.maxIndex;
            else
                max_index=dataset.maxIndex;
        }
        if(sparse_data)
        {
            dataset.denseDataVector = DataTypeConverter::sparseToDense(datasetSparse.rowVector,max_index);
            object.denseDataVector  = DataTypeConverter::sparseToDense(objectSparse.rowVector,max_index);
        }
        else
        {
            datasetSparse.rowVector = DataTypeConverter::denseToSparse(dataset.denseDataVector);
            objectSparse.rowVector  = DataTypeConverter::denseToSparse(object.denseDataVector);
        }
    }

    vector<DenseRecord> D = createRecordVector(dataset.denseDataVector,decisionClassColumn);
    vector<DenseRecord> s = createRecordVector(object.denseDataVector, decisionClassColumn);
    vector<SparseRecord> D_sparse = createRecordVector(datasetSparse.rowVector,decisionClassColumn);
    vector<SparseRecord> s_sparse = createRecordVector(objectSparse.rowVector, decisionClassColumn);

    if(0 == D.size()*s.size() && 0 == D_sparse.size()*s_sparse.size())
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
void ti_l_neighborhood_index_algorithm(vector<data_type> &D, int k, data_type reference_record, vector<data_type> &O)
{
    kni_type kni;
    int number_of_records_in_D = D.size();
    if(terminal_output)
        cout << "Dataset: " << endl;
    for(int i = 0; i<number_of_records_in_D; ++i)
    {
        if(!brute_force)
            D[i].distance = D[i].calculateDistance(reference_record,metric);
        D[i].id=i+1;
        if(terminal_output)
            cout << i << "\t" << D[i] << endl;
    }
    if(terminal_output)
        cout << "Objects: " << endl;
    int number_of_records_in_O = O.size();
    data_type **objects = new data_type*[number_of_records_in_O];
    for(int i = 0; i<number_of_records_in_O; ++i)
    {
        objects[i] = &O[i];
        O[i].distance = O[i].calculateDistance(reference_record,metric);
        O[i].id=-1-i;
        if(terminal_output)
            cout << i << "\t" << O[i] << endl;
    }
    choosenClass.resize(number_of_records_in_O);
    choosenClassTerminal.resize(number_of_records_in_O);
    typename vector<data_type>::iterator object;
    for(int i=0; i<number_of_records_in_O; ++i)
    {
        objectNumber=i;
        D.insert(D.end(),(O.begin()+i),(O.begin()+i+1));
        std::sort(D.begin(),D.end(),comparisonFunction);
        int curr_number_of_records_in_D=D.size();
        for(int i = 0; i<curr_number_of_records_in_D; ++i)
        {
            D[i].position=i;
        }
        object=find(D.begin(),D.end(),*objects[i]);
        kni.insert((*object), ti_k_neighborhood<data_type>(D,(*object),k));
        D.erase(object);
        if(100*i/number_of_records_in_O%2==0)
        {
            cout << "Processing: " << 100*i/number_of_records_in_O << "%";
            if(100*i/number_of_records_in_O<10)
                cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
            else if (100*i/number_of_records_in_O<100)
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
           cout << ii << "# " << O[ii] << endl;
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
            plik << ii << "#" << O[ii] << endl;
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
        cout << "No access to the file!" << endl;
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
    data_type b = p;
    data_type f = p;

    bool backwardSearch = precedingRecord(D, b);
    bool forwardSearch = followingRecord(D, f);

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
bool precedingRecord(vector<data_type> &D, data_type &p)
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
bool followingRecord(vector<data_type> &D, data_type &p)
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
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
                backwardSearch=precedingRecord(D,b);
            }
            else
            {
                distance = f.calculateDistance(p,metric);
                i = i+1;
                data_type e = data_type(f.parameters, distance, f.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
                forwardSearch=followingRecord(D,f);
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
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
                forwardSearch=followingRecord(D,f);
            }
            else
            {
                distance = b.calculateDistance(p,metric);
                i = i+1;
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
                backwardSearch=precedingRecord(D,b);
            }
        }
    }
}
template <class data_type>
void find_first_k_candidate_neighbours_backward(vector<data_type> &D, data_type &p, data_type &b,
                                                bool &backwardSearch, list<data_type> &k_neighborhood, int k, int &i)
{
    float distance;
    while(backwardSearch && i<k)
    {
        distance = b.calculateDistance(p,metric);
        i = i+1;
        data_type e = data_type(b.parameters, distance, b.id);
        typename list<data_type>::iterator it;
        it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
        k_neighborhood.insert(it, e);
        backwardSearch=precedingRecord(D,b);
    }
}
template <class data_type>
void find_first_k_candidate_neighbours_forward(vector<data_type> &D, data_type &p, data_type &f,
                                               bool &forwardSearch, list<data_type> &k_neighborhood, int k, int &i)
{
    float distance;
    while(forwardSearch && i<k)
    {
        distance = f.calculateDistance(p,metric);
        i = i+1;
        data_type e = data_type(f.parameters, distance, f.id);
        typename list<data_type>::iterator it;
        it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
        k_neighborhood.insert(it, e);
        forwardSearch=followingRecord(D,f);
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
            if(k_neighborhood.size() - edge_data_types >= k - 1)
            {
                if(1 < edge_data_types)
                    k_neighborhood.erase(low,up);
                if(1 == edge_data_types)
                    k_neighborhood.erase(low);
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
                p.eps = (*(--k_neighborhood.end())).distance;
            }
            else
            {
                data_type e = data_type(b.parameters, distance, b.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
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
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
            }
        }
        backwardSearch = precedingRecord(D, b);
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
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
                p.eps = (*(--k_neighborhood.end())).distance;
            }
            else
            {
                data_type e = data_type(f.parameters, distance, f.id);
                typename list<data_type>::iterator it;
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
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
                it = find_if(k_neighborhood.begin(), k_neighborhood.end(), firstFurther(e.distance));
                k_neighborhood.insert(it, e);
            }
        }
        forwardSearch = followingRecord(D, f);
    }
}

vector<DenseRecord> createRecordVector(DenseDataVector denseDataVector, int decisionColumnNumber)
{
    vector <DenseRecord> D;
    DenseRecord tmp;
    int i=0;
    for(DenseDataVector::iterator it = denseDataVector.begin(); it != denseDataVector.end(); ++it)
    {
        tmp.parameters.clear();
        for(vector<float>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
        {
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

vector<SparseRecord> createRecordVector(SparseDataVector sparseDataVector, int decisionColumnNumber)
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
            try
            {
                decisionClass.push_back(value);
            }
            catch (const std::invalid_argument&)
            {
                decisionClass.push_back(0);
            }
        }
        col=0;
        ++row;
    }
    file.close();
    return decisionClass;
}
