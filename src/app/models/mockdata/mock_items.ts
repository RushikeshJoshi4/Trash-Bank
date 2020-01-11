import { Item, ItemType } from '../item';

export const ITEMS: Item[] = [
    { ID: 1, title: 'Plastic bottles', ownerID: 1, forSale: true, weight: 10, price: 100, type: ItemType.PLASTIC, path: 'x' },
    { ID: 2, title: 'Paper plates', ownerID: 2, forSale: true, weight: 20, price: 20, type: ItemType.PAPER, path: 'y' },
    { ID: 3, title: 'Paper plates', ownerID: 1, forSale: true, weight: 30, price: 40, type: ItemType.GLASS, path: 'a' },
    { ID: 4, title: 'Paper plates', ownerID: 4, forSale: true, weight: 40, price: 60, type: ItemType.METAL, path: 'b' },
];
