export enum ItemType {
    TRASH = 'trash',
    GLASS = 'glass',
    PLASTIC = 'plastic',
    METAL = 'metal',
    PAPER = 'paper_based',
}

export class Item {
    ID: number;
    ownerID: number;
    forSale: boolean;
    weight: number;
    price: number;
    type: ItemType;
    path: string;
}
