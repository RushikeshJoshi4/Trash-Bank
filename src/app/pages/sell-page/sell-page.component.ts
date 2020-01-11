import { Component, OnInit } from '@angular/core';
import { ItemType } from 'src/app/models/item';

@Component({
  selector: 'app-sell-page',
  templateUrl: './sell-page.component.html',
  styleUrls: ['./sell-page.component.scss']
})
export class SellPageComponent implements OnInit {

  selectedItemType: ItemType;
  ItemType = ItemType;
  constructor() { }

  ngOnInit() {
  }

}
