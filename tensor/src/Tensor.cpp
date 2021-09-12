#include "Tensor.h"

Tensor::Tensor(vector<size_t> _shape, double fill){
    this->shape = _shape;
    totalLength = multiplyArr(shape);
    values.resize(totalLength, fill);
}

Tensor::Tensor(vector<size_t> _shape, vector<double> _values){
    this->shape = _shape;
    totalLength = multiplyArr(shape);
    if(totalLength == _values.size()){
        values.reserve(totalLength);
        for(unsigned i=0; i<totalLength; i++){
            values.push_back(_values[i]);
        }
    }else{
        this->invalidShape();
        std::exit(0);
    }
}

size_t Tensor::multiplyArr(vector<size_t> arr){
    size_t length = 1;
    for(size_t val : arr){
        length = length * val;
    }
    return length;
}

size_t Tensor::complexIndexToLinearIndex(vector<size_t> index){
    if(index.size() > shape.size()){
        this->invalidIndex();
        std::exit(0);
    }
    
    size_t realIndex = 0;
    for(unsigned i=0; i<index.size(); i++){
        if(index[i] >= shape[i]){
            this->invalidIndex();
            std::exit(0);
        }
        realIndex = realIndex + index[i] * multiplyArr(vector<size_t>(shape.begin() + (i + 1), shape.end()));
    }
    return realIndex;
}

//Tensor operations

Tensor Tensor::operator+(Tensor &b){
    if(b.getShape() == shape){
        vector<double> vals(totalLength);
        for(unsigned i=0; i<totalLength; i++){
            vals[i] = values[i] + b.getValues()[i];
        }
        return Tensor(shape, vals);
    }else{
        this->invalidShape();
        std::exit(0);
    }    
}

Tensor Tensor::operator-(Tensor &b){
    if(b.getShape() == shape){
        vector<double> vals(totalLength);
        for(unsigned i=0; i<totalLength; i++){
            vals[i] = values[i] - b.getValues()[i];
        }
        return Tensor(shape, vals);
    }else{
        this->invalidShape();
        std::exit(0);
    } 
}

// Tensor Tensor::operator*(Tensor &b){
//     Tensor multi(*this);

// }

Tensor Tensor::transpose(vector<size_t>){
    Tensor transposed(*this);
    
    return transposed;
}

Tensor Tensor::hadamartProduct(Tensor &b){
    if(b.getShape() == shape){
        vector<double> sum_vals(totalLength);
        for(unsigned i=0; i<totalLength; i++){
            sum_vals[i] = values[i] * b.getValues()[i];
        }
        return Tensor(shape, sum_vals);
    }else{
        this->invalidShape();
        std::exit(0);
    } 
}

// Tensor Tensor::kroneckerMultiplication(Tensor &b){
//     Tensor multi(rows*b.getRows(), cols*b.getCols(), 0.0);
//     unsigned int i, j, p, q;
//     for(i=0; i<rows; i++){
//         for(j=0; j<cols; j++){
//             for(p=0; p<b.getRows(); p++){
//                 for(q=0; q<b.getCols(); q++){
//                     multi(i*b.getRows() + p, j*b.getCols() + q) = this->Tensor[i][j] * b(i, j);
//                 }
//             }
//         }
//     }
//     return multi;
// }

// Tensor Tensor::yConcatonation(Tensor &b){
//     Tensor concat(rows, cols + b.getCols(), 0.0);
//     unsigned int i, j;
//     if(rows == b.getRows()){
//         for(i=0; i<rows; i++){
//             for(j=0; j<cols; j++){
//                 concat(i,j) = this->Tensor[i][j];
//             }
//         }
//         for(i=0; i<b.getRows(); i++){
//             for(j=0; j<b.getCols(); j++){
//                 concat(i,j+cols) = b(i, j);
//             }
//         }
//         return concat;
//     }else{
//         this->invalidShape();
//         std::exit(0);
//     }
// }

Tensor Tensor::flatten(){
    return Tensor({totalLength}, values);
}

Tensor Tensor::reshape(vector<size_t> new_shape){
    if(multiplyArr(new_shape) != totalLength){
        this->invalidShape();
        std::exit(0);
    }
    return Tensor(new_shape, values);
}

double Tensor::max(){
    double max = values[0];
    for(unsigned i=0; i<totalLength; i++){
        if(values[i] > max){
            max = values[i];
        }
    }
    return max;
}

double Tensor::min(){
    double min = values[0];
    for(unsigned i=0; i<totalLength; i++){
        if(values[i] < min){
            min = values[i];
        }
    }
    return min;
}

//Scalar operations

Tensor Tensor::operator+(double scalar){
    vector<double> vals(totalLength);
    for(unsigned i=0; i<totalLength; i++){
        vals[i] = values[i] + scalar;
    }
    return Tensor(shape, vals);
}

Tensor Tensor::operator-(double scalar){
    vector<double> vals(totalLength);
    for(unsigned i=0; i<totalLength; i++){
        vals[i] = values[i] - scalar;
    }
    return Tensor(shape, vals);
}

Tensor Tensor::operator*(double scalar){
    vector<double> vals(totalLength);
    for(unsigned i=0; i<totalLength; i++){
        vals[i] = values[i] * scalar;
    }
    return Tensor(shape, vals);
}

Tensor Tensor::operator/(double scalar){
    vector<double> vals(totalLength);
    for(unsigned i=0; i<totalLength; i++){
        vals[i] = values[i] / scalar;
    }
    return Tensor(shape, vals);
}

// Getters and Setters
vector<size_t> Tensor::getShape(){
    return shape;
}

vector<double>& Tensor::getValues(){
    return values;
}

double& Tensor::operator()(vector<size_t> pos){
    if(pos.size() != shape.size()){
        this->invalidIndex();
        std::exit(0);
    }
    return values[complexIndexToLinearIndex(pos)];
}

Tensor Tensor::operator[](vector<size_t> pos){
    if(pos.size() >= shape.size()){
        this->invalidIndex();
        std::exit(0);
    }
    
    size_t startingIndex = complexIndexToLinearIndex(pos);
    vector<size_t> endingVal(shape.size());
    for(unsigned i=0; i<endingVal.size(); i++){
        if(i < pos.size()){
            endingVal[i] = pos[i];
        }else{
            endingVal[i] = shape[i] - 1;
        }
    }
    size_t endingIndex = complexIndexToLinearIndex(endingVal);
    vector<double> _values = vector<double>(values.begin() + startingIndex, values.begin() + endingIndex + 1);
    vector<size_t> _shape = vector<size_t>(shape.begin() + pos.size(), shape.end());
    return Tensor(_shape, _values);
}

//Other methods
void Tensor::printShape(){
    //Print shape
    unsigned i, j;
    std::cout << "Tensor shape: ";
    for(i=0; i<shape.size(); i++){
        std::cout << shape[i];
        if(i < shape.size() - 1){
            std::cout << " x ";
        }
    }
    std::cout << std::endl << std::endl;
}

void Tensor::invalidShape(){
    try{ 
        throw std::length_error("Provided tensor have invalid shape, so operation can't be done"); 
    }
    catch(std::length_error str){
        std::cout << str.what() << std::endl; 
    }
}

void Tensor::invalidIndex(){
    try{ 
        throw std::out_of_range("Provided index is not in bounds of specified tensor"); 
    }
    catch(std::out_of_range str){
        std::cout << str.what() << std::endl; 
    }
}