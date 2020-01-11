import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { User } from '../models/user';
import { Observable } from 'rxjs';
import { environment } from 'src/environments/environment';

@Injectable({
  providedIn: 'root'
})
export class ItemService {

  constructor(private http: HttpClient) {}

  getItemsForSale(user: User): Observable<any> {
    const data = {user_id: user.ID, address: user.address};
    return this.http.post(environment.ITEMS_URL, data);
  }
}
