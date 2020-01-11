import { Component, OnInit } from '@angular/core';
import { Item, ItemType } from 'src/app/models/item';
import { ITEMS } from 'src/app/models/mockdata/mock_items';

@Component({
  selector: 'app-buy-page',
  templateUrl: './buy-page.component.html',
  styleUrls: ['./buy-page.component.scss']
})
export class BuyPageComponent implements OnInit {

  items: Item[];
  constructor() {
    this.items = [];
  }

  ngOnInit() {
    this.items = ITEMS;
  }

  getCategoryIcon(type: ItemType) {
    const obj = {
      'fa-trash-alt': type === ItemType.TRASH,
      'fa-glass-whiskey': type === ItemType.GLASS,
      'fa-shopping-bag': type === ItemType.PLASTIC,
      'fa-shield-alt': type === ItemType.METAL,
      'fa-copy': type === ItemType.PAPER,
    };
    console.log(obj);
    return obj;
  }

}
