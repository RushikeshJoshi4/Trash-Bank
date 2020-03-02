import { Component, OnInit } from '@angular/core';
import { ItemType } from 'src/app/models/item';
import { ItemService } from 'src/app/services/item.service';
import { LoginService } from 'src/app/services/login.service';
import { User } from 'src/app/models/user';
import { UtilityService } from 'src/app/services/utility.service';
declare var $: any;

@Component({
  selector: 'app-sell-page',
  templateUrl: './sell-page.component.html',
  styleUrls: ['./sell-page.component.scss']
})
export class SellPageComponent implements OnInit {

  // selectedItemType: ItemType;
  uploadLabel: string;
  formdata: any;
  currentUser: User;
  ItemType = ItemType;

  constructor(private itemService: ItemService, private loginService: LoginService, private utility: UtilityService) {
    this.uploadLabel = 'Upload photo of product to predict fields below!';
    this.formdata = {
      item_type: undefined,
    };
  }

  ngOnInit() {
    this.loginService.getUserDetails().subscribe(uresp => {
      if (uresp instanceof User) {
        this.currentUser = uresp;
      } else {
        this.loginService.user = new User(uresp);
        this.currentUser = this.loginService.user;
      }

      this.formdata = {
        user_id: this.currentUser.ID,
        address: this.currentUser.address,
        title: '',
        weight: 0,
        price: 0,
        item_type: undefined,
        image: undefined,
      };
    });

  }

  pickImage(event) {
    const file: File = event.target.files[0];
    this.uploadLabel = file.name;
    this.formdata.image = file;

    // Turning on prediction
    // $('#preventor').css('display', 'flex');

    // // Sneak
    // setTimeout(() => {
    //   $('#preventor').css('display', 'none');
    //   this.formdata.item_type = 'Metal';
    //   this.formdata.weight = 347;
    //   this.formdata.price = 1;
    // }, 5000);
  }

  submit() {
    this.utility.displayLoading('Adding item...');
    console.log('sell form', this.formdata);
    this.itemService.addItem(this.formdata).subscribe(addresp => {
      console.log('add', addresp);
      this.formdata = {
        user_id: this.currentUser.ID,
        address: this.currentUser.address,
        title: '',
        weight: 0,
        price: 0,
        item_type: undefined,
        image: undefined,
      };
      this.utility.hideLoading();
    });
  }

  test(x) {
    console.log(x);
  }

}
