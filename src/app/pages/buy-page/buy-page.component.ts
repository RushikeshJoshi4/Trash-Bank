import { Component, OnInit } from '@angular/core';
import { Item, ItemType } from 'src/app/models/item';
import { ITEMS } from 'src/app/models/mockdata/mock_items';
import { LoginService } from 'src/app/services/login.service';
import { ItemService } from 'src/app/services/item.service';
import { UtilityService } from 'src/app/services/utility.service';
import { User } from 'src/app/models/user';

@Component({
  selector: 'app-buy-page',
  templateUrl: './buy-page.component.html',
  styleUrls: ['./buy-page.component.scss']
})
export class BuyPageComponent implements OnInit {

  items: Item[];
  typeFilter: ItemType;
  currentUser: User;
  ItemType = ItemType;

  constructor(private login: LoginService, private itemService: ItemService, private utility: UtilityService) {
    this.items = [];
  }

  ngOnInit() {
    this.login.getUserDetails().subscribe(uresp => {
      if (uresp instanceof User) {
        this.currentUser = uresp;
      } else {
        this.login.user = new User(uresp);
        this.currentUser = this.login.user;
      }

      this.itemService.getItemsForSale(this.currentUser).subscribe(iresp => {
        console.log('buy', iresp);
        this.items = Item.parseTransacArr(iresp.data);
        // this.forSaleData = tresp.items_for_sale;
        this.utility.hideLoading(1000);
      });
    });

    // this.items = ITEMS;
  }

  getCategoryIcon(type: ItemType) {
    const obj = {
      // 'fa-trash-alt': type === ItemType.TRASH,
      'fa-glass-whiskey': type === ItemType.GLASS,
      'fa-shopping-bag': type === ItemType.PLASTIC,
      'fa-shield-alt': type === ItemType.METAL,
      'fa-copy': type === ItemType.PAPER,
    };
    return obj;
  }

  test() {
    console.log(this.typeFilter);
  }

}
