import { Injectable } from '@angular/core';
declare var $: any;

@Injectable({
  providedIn: 'root'
})
export class UtilityService {

  constructor() { }

  displayLoading(msg?: string) {
    msg = msg ? msg : 'Loading...';
    // console.log(msg);
    $('#loading-modal').modal('show');
    $('#load-msg').innerText = msg;
  }

  hideLoading(delay?: number) {
    delay = delay ? delay : 1000;
    // console.log(delay);
    setTimeout(() => {
      $('#loading-modal').modal('hide');
    }, delay);
  }
}
