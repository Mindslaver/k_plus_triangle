#include "SparseDataLoader.h"

void SparseDataLoader::readSparseData(std::string filename){

    std::ifstream file (filename);
	std::string temp;
	std::getline(file, temp);
    while (std::getline(file, temp))
    {
        std::istringstream buffer(temp);
        std::vector<int> tmpLine((std::istream_iterator<int>(buffer)), std::istream_iterator<int>());
        std::vector<std::pair<int,float>> tmpVector;

        for(std::vector<int>::iterator it = tmpLine.begin(); it != tmpLine.end();++it)
        {
            int index = *it;
            if(index > maxIndex)
                maxIndex = index;
            ++it;
            tmpVector.push_back(std::make_pair(index,(float) *it));
        }
        this->rowVector.push_back(tmpVector);
    }
	file.close();
}

void SparseDataLoader::displayRow(){
        for(SparseDataVector::iterator it = rowVector.begin(); it != rowVector.end(); ++it)
        {
            for(std::vector<std::pair<int,float>>::iterator rowIt = (*it).begin(); rowIt != (*it).end(); ++rowIt)
            {
                std::cout << "Index slowa: " << (*rowIt).first << " liczba wystapien " << (*rowIt).second <<" ";
            }
        std::cout << std::endl;
	}
}

SparseDataVector SparseDataLoader::getRowVector()
{
    return rowVector;
}
