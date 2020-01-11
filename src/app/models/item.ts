export enum ItemType {
    // TRASH = 'trash',
    GLASS = 'Glass',
    PLASTIC = 'Plastic',
    METAL = 'Metal',
    PAPER = 'Paper',
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
