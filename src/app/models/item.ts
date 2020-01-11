export enum ItemType {
    TRASH = 'trash',
    GLASS = 'glass',
    PLASTIC = 'plastic',
    METAL = 'metal',
    PAPER = 'paper_based',
}

export class Item {
    ID: number;
    title: string;
    ownerName: string;
    forSale: boolean;
    weight: number;
    price: number;
    type: ItemType;
    path: string;
}
