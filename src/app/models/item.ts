export enum ItemType {
    // TRASH = 'trash',
    GLASS = 'Glass',
    PLASTIC = 'Plastic',
    METAL = 'Metal',
    PAPER = 'Paper',
}

// let BACKEND_MAP = {
//     'PAPER':
// }

export class Item {
    ID: number;
    title: string;
    ownerName: string;
    // forSale: boolean;
    weight: number;
    price: number;
    type: ItemType;
    path: string;

    constructor(i) {
        this.ID = i.item_id;
        this.title = i.item_title;
        this.ownerName = i.item_owner;
        this.weight = i.item_weight;
        this.price = i.item_price;
        this.type = i.item_type;
        this.path = i.item_path;
    }
}
