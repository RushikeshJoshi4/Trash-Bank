import { Item, ItemType } from '../item';

export const MKITEMS: Item[] = [
    { ID: 1, ownerID: 1, forSale: true, weight: 10, price: 100, type: ItemType.PLASTIC, path: 'x' },
    { ID: 2, ownerID: 2, forSale: false, weight: 50, price: 20, type: ItemType.PAPER, path: 'y' }
];
