#include "DBLF.hpp"

void DBLF(
    Bin& bin,
    Chromosome itemsIdToPack, 
    Chromosome itemsRotation, 
    LoadedBins itemDataOrderById){
    ROTATION_WAY rotationWay = bin.getRotationWay();
    ColaPuntosDBL queue = ColaPuntosDBL::Build();
    queue.push(Punto::Build(0,0,0));

    for(int i = 0; i < itemsIdToPack.size(); i++){
        int boxId = itemsIdToPack[i];
        ROTATION_MODE rotation = getRotationFromId(itemsRotation[i]);
        ItemBin itemToPack = itemDataOrderById[i].rotate(rotationWay, rotation); 
        if (!itemToPack.isValidOrientation()) {
            continue;
        }
        for(int j = 0; j < queue.size(); j++){
            Punto currenPoint = queue[i];
            Punto dimensionPoint;
            dimensionPoint.x = bin.getDimensions().getLargo();
            dimensionPoint.y = bin.getDimensions().getAncho();
            dimensionPoint.z = bin.getDimensions().getAlto();
            if (canBePlaced(bin, currenPoint + dimensionPoint)) {
                bool isOverlap = isOverlapped(queue[i], itemToPack, itemDataOrderById, bin, rotation);
                if (!isOverlap){
                    queue.remove(j);
                    addItemToBin(queue, bin, queue[i], itemToPack, itemDataOrderById, rotation);
                    break;
                }
            }
        }
    }
}