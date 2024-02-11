#include "ItemBinOperators.hpp"

bool canBePlaced(Bin bin,const Punto &possiblePoint){
    return possiblePoint.z <= bin.getDimensions().getAlto() &&
           possiblePoint.y <= bin.getDimensions().getAncho() &&
           possiblePoint.x <= bin.getDimensions().getLargo();
}

bool isABIntersect(Punto Amax, Punto Amin, Punto Bmax, Punto Bmin){
    return Amin.x < Bmax.x && Amax.x > Bmin.x 
        && Amin.y < Bmax.y && Amax.y > Bmin.y
        && Amin.z < Bmax.z  && Amax.z > Bmin.z;
}
//Min O(1), Max O(N), Prom O(N/2)
bool isOverlapped(Punto position, ItemBin currentBin, LoadedBins dataSet, Bin bin, ROTATION_MODE rotation){
    for(int i = 0; i < bin.getNumberOfLoadedItems() ; i++){
        Punto Bmin = bin.getLoadedItems()[i].getPunto();
        //Verifica si la caja tiene su respectiva rotacion.
        ItemBin oldItem = dataSet[bin.getLoadedItems()[i].getId() - 1];
        oldItem = oldItem.rotate(bin.getRotationWay(), rotation);

        if (isABIntersect(
                /* Amax= */ position, 
                /* Amin= */ position + currentBin.getCurrentDimension(),
                /* Bmax= */ oldItem.getCurrentDimension(), 
                /* Bmin= */ Bmin + oldItem.getCurrentDimension())){
            return true;
        }
    }

    return false;
}

//Min O(1), Max O(N2), Prom O(N2)
void iterateByDeepestBottomLeft(Punto& punto, ItemBin item, LoadedBins dataSet, Bin bin, ROTATION_MODE rotationWay){
    if (bin.getNumberOfLoadedItems() <= 1){
        return;
    }
    Punto copyPoint = punto;
    //Go first by Deepest - X
    if (punto.x != 0){
        while (!isOverlapped(copyPoint, item, dataSet, bin, rotationWay))
        {
           punto.x -= 1;
           if (punto.x == 0) {
            break;
           }
        }
    }
    //Go Second by Bottom - Z
    if (punto.z != 0){
        while (!isOverlapped(punto, item, dataSet, bin, rotationWay))
        {
           punto.z -= 1;
           if (punto.z == 0) {
            break;
           }
        }
    }
    //Go Thirsd by With - Y
    if (punto.y != 0){
        while (!isOverlapped(punto, item, dataSet, bin, rotationWay))
        {
           punto.y -= 1;
           if (punto.y == 0) {
            break;
           }
        }
    }
}
void addItemToBin(ColaPuntosDBL& queue, Bin& bin, Punto& punto, ItemBin item, LoadedBins idBins, ROTATION_MODE rotationMode){
    iterateByDeepestBottomLeft(punto, item, idBins, bin, rotationMode);
    item.rotate(bin.getRotationWay(), rotationMode);
    bin.addItem(item);
    Punto resultPoint = punto + item.getCurrentDimension();
    Punto newPoint;
    if ( resultPoint.x < bin.getDimensions().getLargo()){
        newPoint = Punto::Build(resultPoint.x, punto.y, punto.z);
        queue.push(newPoint);
    }
    if ( resultPoint.y < bin.getDimensions().getAncho()){
         newPoint = Punto::Build(punto.x, resultPoint.y, punto.z);
        queue.push(newPoint);
    }
    if ( resultPoint.z < bin.getDimensions().getAlto()){\
         newPoint = Punto::Build(punto.x, punto.y, resultPoint.z);
        queue.push(newPoint);
    }
    queue.update();
}

