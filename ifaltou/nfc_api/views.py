from django.shortcuts import render
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
import json

# def receber_json(request):
#     if request.method == 'POST':
#         # Recebendo e processando o JSON recebido no corpo da requisição
#         data = json.loads(request.body)
        
#         # Faça o que você precisa fazer com os dados aqui
#         # Por exemplo, você pode salvar os dados no banco de dados
        
#         # Retorne uma resposta, por exemplo, confirmando que os dados foram recebidos
#         return JsonResponse({'mensagem': 'Dados JSON recebidos com sucesso!'}, data)
#     else:
#         # Se a requisição não for POST, retorne uma resposta de erro
#         return JsonResponse({'erro': 'Apenas requisicoes POST sao permitidas.'}, status=400)
@csrf_exempt
def receber_json(request):
    if request.method == 'POST':
        tag = request.GET.get('tag') 
        print("TAG: " + str(tag))
    
        print(request.body)
        try:
            # Obtendo os dados JSON da requisição POST
            data = json.loads(request.body)
            tag = request.GET.get('tag')
            
            print("Aqui está a TAG: " + str(tag))
            
            tag_value = data.get('tag_value', '')
            print("Aqui está a TAG Value: " + str(tag_value))
            # Aqui você pode processar os dados conforme necessário
            # Por exemplo, você pode salvar os dados no banco de dados
            # ou realizar outras operações comerciais

            # Enviando uma resposta de sucesso
            response_data = {'status': 'success', 'message': 'Dados recebidos com sucesso', 'tag': tag}
            return JsonResponse(response_data)
        except json.JSONDecodeError as e:
            # Lidar com erros de decodificação JSON, se necessário
            response_data = {'status': 'error', 'message': f'Erro na decodificacao JSON: {str(e)}'}
            return JsonResponse(response_data, status=400)
    else:
        # Se a requisição não for POST, retornar um erro
        response_data = {'status': 'error', 'message': 'Apenas requisições POST são suportadas'}
        return JsonResponse(response_data, status=405)