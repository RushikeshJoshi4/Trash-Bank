import { Item } from './item';

export enum TransactionStatus {
    PENDING,
    COMPLETED
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
}
