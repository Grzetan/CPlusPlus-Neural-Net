#include "Tensor.h"

Tensor::Tensor(){
    
}

Tensor::Tensor(vector<size_t> _shape, double fill){
    this->shape = _shape;
    this->generateStrides();
    totalLength = multiplyArr(shape);
    values.resize(totalLength, fill);
}

Tensor::Tensor(vector<size_t> _shape, vector<double> _values, vector<size_t> _strides = {}, bool _contiguous = true){
    this->shape = _shape;
    this->contiguous = _contiguous;
    if(_strides.size() == 0){
        this->generateStrides();
    }else{
        this->strides = _strides;
    }
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
        realIndex = realIndex + index[i] * strides[i];
    }
    return realIndex;
}

vector<size_t> Tensor::linearToComplexIndex(size_t index){
    vector<size_t> complex(shape.size());
    vector<size_t> tempStrides;

    if(!contiguous){
        tempStrides.reserve(shape.size());
        for(int i=0; i<shape.size(); i++){
            tempStrides.push_back(this->multiplyArr(vector<size_t>(shape.begin() + i + 1, shape.end())));
        }
    }else{
        tempStrides = strides;
    }
    unsigned leftover = index;
    for(unsigned i=0; i<shape.size(); i++){
        complex[i] = leftover / tempStrides[i];
        leftover = leftover % tempStrides[i];
    }
    return complex;
}

void Tensor::generateStrides(){
    strides.reserve(shape.size());
    for(int i=0; i<shape.size(); i++){
        strides.push_back(this->multiplyArr(vector<size_t>(shape.begin() + i + 1, shape.end())));
    }
}

vector<double> Tensor::copyValuesByStrides(){
    vector<double> _values(totalLength);
    for(unsigned i=0; i<totalLength; i++){
        _values[i] = this->operator()(this->linearToComplexIndex(i));
    }
    return _values;
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
        vector<double> _vals = b.getValues();
        for(unsigned i=0; i<totalLength; i++){
            vals[i] = values[i] - _vals[i];
        }
        return Tensor(shape, vals);
    }else{
        this->invalidShape();
        std::exit(0);
    } 
}

Tensor Tensor::operator*(Tensor &b){
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

Tensor Tensor::swapaxes(size_t dim1, size_t dim2){
    //Create new shape
    vector<size_t> _shape = shape; 
    _shape[dim2] = shape[dim1];
    _shape[dim1] = shape[dim2];
    //Swap values
    vector<size_t> _strides = strides;
    _strides[dim1] = strides[dim2];
    _strides[dim2] = strides[dim1];
    return Tensor(_shape, values, _strides, false);
}

Tensor Tensor::transpose(){
    vector<size_t> newShape;
    vector<size_t> newStrides;
    newShape.reserve(shape.size());
    newStrides.reserve(strides.size());

    for(int i=shape.size() - 1; i>=0; i--){
        newShape.push_back(shape[i]);
        newStrides.push_back(strides[i]);
    }
    return Tensor(newShape, values, newStrides, false);
}

Tensor Tensor::permute(vector<size_t> dims){
    if(dims.size() == shape.size()){
        vector<size_t> newShape;
        vector<size_t> newStrides;
        newShape.reserve(shape.size());
        newStrides.reserve(strides.size());
        
        for(unsigned i=0; i<dims.size(); i++){
            if(dims[i] >= shape.size()){
                this->invalidIndex();
                std::exit(0);   
            }
            newShape.push_back(shape[dims[i]]);
            newStrides.push_back(strides[dims[i]]);
        }
        return Tensor(newShape, values, newStrides, false);
    }else{
        this->invalidIndex();
        std::exit(0);
    }
}

Tensor Tensor::matmul(Tensor &b){
    if(shape[shape.size() - 1] == b.getShape()[b.getShape().size() - 2] && shape.size() == 2 && b.getShape().size() == 2){
        vector<size_t> _shape = shape;
        _shape[shape.size() - 1] = b.getShape()[shape.size() - 1];
        vector<double> _values;
        _values.reserve(multiplyArr(_shape));
        unsigned i,j,k;
        double sum;
        for(i=0; i<shape[0]; i++){
            for(j=0; j<b.getShape()[1]; j++){
                sum = 0;
                for(k=0; k<shape[1]; k++){
                    sum += this->operator()({i,k}) * b({k,j});
                }
                _values.push_back(sum);
            }
        }
        return Tensor(_shape, _values);
    }else{
        std::cout << "Make sure that tensors have 2 dimensions and column count in first tensor == row count in second tensor" << std::endl;
        std::exit(0);
    }
}

Tensor Tensor::kron(Tensor &b){
    //Check if tensors have equal shapes
    if(shape.size() != b.getShape().size()){
        std::cout << "To perform kronecker multiplication tensors must have the same number of dimensions" << std::endl;
        std::exit(0);
    }

    //Calculate new shape
    unsigned i, j, k, p;
    vector<size_t> _shape = shape;
    vector<size_t> other = b.getShape();

    int max = shape.size();
    if(b.getShape().size() > max){
        max = b.getShape().size();
    }
    for(i=0; i<max - _shape.size(); i++){
        _shape.insert(_shape.begin(), 1);
    }
    for(i=0; i<max - other.size(); i++){
        other.insert(other.begin(), 1);
    }
    for(i=0; i<_shape.size(); i++){
        _shape[i] = _shape[i] * other[i];
    }    

    Tensor result(_shape, 0);
    vector<size_t> aComplexIndex(shape.size()), blockIndex(shape.size()), bComplexIndex(shape.size()), resultIndex(shape.size());
    for(i=0; i<totalLength; i++){
        aComplexIndex = linearToComplexIndex(i);
        for(j=0; j<aComplexIndex.size(); j++){
            blockIndex[j] = aComplexIndex[j] * other[j];
        }
        for(k=0; k<b.totalLength; k++){
            bComplexIndex = b.linearToComplexIndex(k);
            resultIndex = blockIndex;
            for(p=0; p<bComplexIndex.size(); p++){
                resultIndex[p] += bComplexIndex[p];
            }
            result(resultIndex) = values[i] * b.getValues()[k];
        }
    }

    return result;
}

Tensor Tensor::concat(Tensor &b, size_t axis = 0){
    //Check if axis is valid
    if(axis < 0 || axis >= shape.size()){
        this->invalidAxis();
        std::exit(0);
    }
    //Check if tensors can be concatonated
    unsigned i, j;
    for(i=0; i<shape.size(); i++){
        if(shape[i] != b.getShape()[i] && i != axis){
            this->invalidShape();
            std::exit(0);
        }    
    }
    //Concatonate tensors
    vector<size_t> _shape = shape;
    _shape[axis] += b.getShape()[axis];
    vector<double> _values;
    _values.reserve(totalLength + b.totalLength);

    vector<double> copiedValues = values;
    vector<double> copiedValuesB = b.getValues();
    if(!contiguous){
        copiedValues = this->copyValuesByStrides();
    }
    if(!b.contiguous){
        copiedValuesB = b.copyValuesByStrides();
    }
    size_t stepA = multiplyArr(vector<size_t>(shape.begin() + axis, shape.end()));
    vector<size_t> shapeB = b.getShape();
    size_t stepB = multiplyArr(vector<size_t>(shapeB.begin() + axis, shapeB.end()));
    size_t range = multiplyArr(vector<size_t>(shape.begin(), shape.begin() + axis));
    for(i=0; i<range; i++){
        for(j=i*stepA; j<i*stepA+stepA; j++){
            _values.push_back(copiedValues[j]);
        }
        for(j=i*stepB; j<i*stepB+stepB; j++){
            _values.push_back(copiedValuesB[j]);
        }
    }
    return Tensor(_shape, _values);
}

Tensor Tensor::flatten(){
    vector<double> _values = values;
    if(!contiguous){
        _values = this->copyValuesByStrides();
    }
    return Tensor({totalLength}, _values);
}

Tensor Tensor::reshape(vector<size_t> new_shape){
    if(multiplyArr(new_shape) != totalLength){
        this->invalidShape();
        std::exit(0);
    }
    vector<double> _values = values;
    if(!contiguous){
        _values = this->copyValuesByStrides();
    }
    return Tensor(new_shape, _values);
}

Tensor Tensor::unsquezze(){
    vector<double> _values;    

    if(!contiguous){
        _values = copyValuesByStrides();
    }else{
        _values = values;
    }

    vector<size_t> _shape = shape;
    _shape.insert(_shape.begin(), 1);

    return Tensor(_shape, _values);
}

Tensor Tensor::squezze(){
    if(shape[0] == 1){
        vector<double> _values;    

        if(!contiguous){
            _values = copyValuesByStrides();
        }else{
            _values = values;
        }

        vector<size_t> _shape = shape;
        _shape.erase(_shape.begin());

        return Tensor(_shape, _values);
    }else{
        std::cout << "Cannot squeeze tensor if first dimension != 1" << std::endl;
        std::exit(0);
    }
}

Tensor Tensor::square(){
    vector<double> _values = values;
    for(unsigned i=0; i<totalLength; i++){
        _values[i] = _values[i] * _values[i];
    }
    return Tensor(shape, _values);
}

double Tensor::sum(){
    double sum = 0;
    for(unsigned i=0; i<totalLength; i++){
        sum += values[i];
    }
    return sum;
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

size_t Tensor::dimCount(){
    return shape.size();
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

void Tensor::insertTensor(vector<size_t> index, Tensor& b){
    if(b.getShape() != vector<size_t>(shape.begin() + index.size(), shape.end())){
        this->invalidIndex();
        std::exit(0);
    }else{
        size_t startIndex = this->complexIndexToLinearIndex(index);
        for(unsigned i=0; i<b.totalLength; i++){
            values[startIndex + i] = b.getValues()[i];
        }
    }
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

void Tensor::printStrides(){
    //Print shape
    unsigned i, j;
    std::cout << "Tensor strides: ";
    for(i=0; i<strides.size(); i++){
        std::cout << strides[i];
        if(i < strides.size() - 1){
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

void Tensor::invalidAxis(){
    try{ 
        throw std::out_of_range("Provided axis is invalid"); 
    }
    catch(std::out_of_range str){
        std::cout << str.what() << std::endl; 
    }
}