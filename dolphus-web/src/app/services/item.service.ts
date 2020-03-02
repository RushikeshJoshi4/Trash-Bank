import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { User } from '../models/user';
import { Observable } from 'rxjs';
import { environment } from 'src/environments/environment';

@Injectable({
  providedIn: 'root'
})
export class ItemService {

  constructor(private http: HttpClient) { }

  getItemsForSale(user: User): Observable<any> {
    const data = { user_id: user.ID, address: user.address };
    return this.http.post(environment.ITEMS_URL, data);
  }

  addItem(itemObj): Observable<any> {
    const formData = new FormData();
    formData.set('user_id', itemObj.user_id);
    formData.set('address', itemObj.address);
    formData.set('title', itemObj.title);
    formData.set('weight', itemObj.weight);
    formData.set('price', itemObj.price);
    formData.set('item_type', itemObj.item_type);
    formData.set('image', itemObj.image);
    // for (const key in itemObj) {
    //   if (itemObj.hasOwnProperty(key)) {
    //     const value = itemObj.key;
    //     formData.set(key, value);
    //   }
    // }
    return this.http.post(environment.ADD_URL, formData);
  }

  buyItem(user: User, itemID: number) {
    const obj = {
      user_id: user.ID,
      item_id: itemID,
      address: user.address,
    };
    return this.http.post(environment.BUY_URL, obj);
  }
}
