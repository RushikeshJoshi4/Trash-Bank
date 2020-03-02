import { Item } from './item';

export enum TransactionStatus {
    SALE = 'For Sale',
    PENDING = 'Awaiting Delivery',
    COMPLETED = 'Complete'
}

export class Transaction {
    ID: number;
    // buyerID: number;
    // sellerID: number;
    otherParty: string;
    isSeller: boolean;
    item: Item;
    status: TransactionStatus;
    datetime: Date;

    constructor(t) {
        this.ID = t.transac_id;
        this.isSeller = t.is_seller as boolean;
        if (this.isSeller) {
            this.otherParty = t.buyer;
        } else {
            this.otherParty = t.seller;
        }
        this.item = new Item(t.item);
        this.status = t.status;
        this.datetime = new Date(t.date);
    }

    static parseTransacArr(ts): Transaction[] {
        const transacs: Transaction[] = [];
        for (const t of ts) {
            transacs.push(new Transaction(t));
        }
        return transacs;
    }
}
