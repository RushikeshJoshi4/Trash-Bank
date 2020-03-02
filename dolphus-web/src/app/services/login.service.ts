import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable, of } from 'rxjs';
import { environment } from 'src/environments/environment';
import { User } from '../models/user';
import { UtilityService } from './utility.service';

@Injectable({
  providedIn: 'root'
})
export class LoginService {

  user: User;

  constructor(private http: HttpClient, private utility: UtilityService) {
  }

  getUserDetails(uname?: string): Observable<any> {
    if (uname) {
      const data = {
        uname,
        address: -1
      };
      return this.http.post(environment.LOGIN_URL, data);
    }
    if (localStorage.getItem('uname')) {
      uname = localStorage.getItem('uname');
    }
    if (!this.user) {
      uname = uname ? uname : 'Erica';
      const data = {
        uname,
        address: -1
      };
      return this.http.post(environment.LOGIN_URL, data);
    } else {
      return of(this.user);
    }
  }

}
