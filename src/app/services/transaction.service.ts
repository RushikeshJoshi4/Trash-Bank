import { Injectable } from '@angular/core';
import { User } from '../models/user';
import { HttpClient } from '@angular/common/http';
import { environment } from 'src/environments/environment';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class TransactionService {

  constructor(private http: HttpClient) { }

  getTransactionsOfUser(user: User): Observable<any> {
    const obj = { user_id: user.ID, address: user.address };
    return this.http.post(environment.TRANSAC_URL, obj);
  }
}
