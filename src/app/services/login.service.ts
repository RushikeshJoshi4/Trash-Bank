import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';
import { environment } from 'src/environments/environment';
import { User } from '../models/user';

@Injectable({
  providedIn: 'root'
})
export class LoginService {

  user: User;

  constructor(private http: HttpClient) {
    this.getUserDetails().subscribe(resp => {
      this.user = new User(resp);
      console.log(this.user);
    });
  }

  getUserDetails(uname?: string): Observable<any> {
    uname = uname ? uname : 'ankitesh';
    const data = {
      uname,
      address: -1
    };
    return this.http.post(environment.LOGIN_URL, data);
  }

}
