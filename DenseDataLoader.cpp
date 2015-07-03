#include "DenseDataLoader.h"
#include <stdlib.h>

void DenseDataLoader::readDenseData(std::string filename)
{
    std::ifstream file (filename);
    std::string line;
	getline(file,line);
	int row = 0;
    while(getline(file,line))
    {
	    std::stringstream   linestream(line);
	    std::string value;
	    denseDataVector.resize(row + 1);
        while(getline(linestream,value,','))
        {
            try
            {
                float tmp = strtof(value.c_str(),0);
				denseDataVector[row].push_back(tmp);
            }
            catch (const std::invalid_argument&)
            {
				denseDataVector[row].push_back(0);
			}	        
	    }
	    ++row;
	}
	file.close();
}

void DenseDataLoader::displayData(){
    for(DenseDataVector::iterator it = denseDataVector.begin(); it != denseDataVector.end(); ++it)
    {
        for(std::vector<float>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
        {
            std::cout << *it2 << ", ";
		}
        std::cout << std::endl;
	}	
}
